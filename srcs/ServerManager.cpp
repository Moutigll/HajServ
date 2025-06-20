/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:52:01 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/20 15:37:09 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerManager.hpp"

volatile sig_atomic_t keepRunning = 1;

void signalHandler(int signal)
{
    if (signal == SIGINT)
	{
        std::cout << "\nCtrl+C detected. Exiting main loop...\n";
        keepRunning = false;
    }
}

ServerManager::ServerManager(void)
{
	this->_epollFd = -1;
}

ServerManager::ServerManager(const ServerManager &other)
{
	(void)other;
	this->_epollFd = -1;
}

ServerManager &ServerManager::operator=(const ServerManager &other)
{
	(void)other;
	return (*this);
}

ServerManager::~ServerManager(void)
{
	std::map<int, Connection *>::iterator	it;

	for (it = this->_connections.begin();
		it != this->_connections.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	this->_connections.clear();
	for (size_t i = 0; i < this->_ports.size(); ++i)
		delete this->_ports[i];
	if (this->_epollFd != -1)
	close(this->_epollFd);
}

bool	ServerManager::addToEpoll(int fd, uint32_t events)
{
	struct epoll_event	event;

	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		g_logger.log(LOG_ERROR, "Failed to add fd " + to_string(fd) + " to epoll: " + std::string(strerror(errno)));
		return false;
	}
	return true;
}

bool	ServerManager::init(const std::vector<t_server> &servers)
{
	std::map<int, Port *>	port_map;
	std::map<int, std::vector<t_server const *> >	port_to_servers;

	this->_epollFd = epoll_create1(0);
	if (this->_epollFd == -1)
	{
		g_logger.log(LOG_ERROR, "Failed to create epoll instance: " + std::string(strerror(errno)));
		return false;
	}

	for (std::vector<t_server>::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		for (std::vector<int>::const_iterator pit = it->_ports.begin(); pit != it->_ports.end(); ++pit)
			port_to_servers[*pit].push_back(&(*it));
	}

	for (std::map<int, std::vector<t_server const *> >::iterator it = port_to_servers.begin(); it != port_to_servers.end(); ++it)
	{
		int	port_number = it->first;

		Port *port = new Port(port_number, NULL);

		for (std::vector<t_server const *>::iterator sit = it->second.begin(); sit != it->second.end(); ++sit)
			port->addServer(const_cast<t_server *>(*sit));

		if (!port->init())
		{
			delete port;
			continue;
		}

		this->_ports.push_back(port);

		if (!this->addToEpoll(port->getSocketFd(), EPOLLIN))
		{
			delete port;
		}
	}

	return true;
}


/*-------------------
    Main logic loop
---------------------*/

Port *ServerManager::isListeningSocket(int fd) const
{
	for (size_t i = 0; i < this->_ports.size(); ++i)
	{
		if (this->_ports[i]->getSocketFd() == fd)
			return this->_ports[i];
	}
	return NULL;
}

void	ServerManager::start(void)
{
	struct epoll_event	events[EPOLL_MAX_EVENTS];
	int					nfds;
	int					i;

	std::signal(SIGINT, signalHandler);
	
	g_logger.log(LOG_INFO, "Server started, waiting for events...");

	while (keepRunning)
	{
		nfds = epoll_wait(this->_epollFd, events, EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);
		if (nfds == -1)
		{
			if (errno == EINTR)
				continue;
			g_logger.log(LOG_ERROR, "epoll_wait failed: " + std::string(strerror(errno)));
			break;
		}

		i = 0;
		while (i < nfds)
		{
			int	fd = events[i].data.fd;
			Port *port = this->isListeningSocket(fd);

			if (port)
				newConnection(port);
			else
				handleConnectionEvent(events[i]);
			++i;
		}
		this->checkTimeouts();
	}
}

void	ServerManager::newConnection(Port *port)
{
	if (port == NULL || port->getSocketFd() < 0)
	{
		g_logger.log(LOG_ERROR, "Invalid port for new connection");
		return;
	}

	int client_fd = accept(port->getSocketFd(), NULL, NULL);
	if (client_fd < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			g_logger.log(LOG_ERROR, "Failed to accept new connection on fd " + to_string(port->getSocketFd()) + ": " + std::string(strerror(errno)));
		return;
	}

	Connection *conn = new Connection(client_fd, port);
	if (conn->isClosed())
	{
		close(client_fd);
		delete conn;
		return;
	}

	this->_connections[client_fd] = conn;

	/*
	* EPOLLIN: Indicates that the file descriptor is ready for reading.
	* EPOLLET: Edge-triggered mode, meaning events are reported only when the state
	*          changes, not continuously while the condition is true.
	* EPOLLRDHUP: Indicates that the remote end has closed the connection.
	* EPOLLHUP: Indicates that the file descriptor has been hung up.
	* EPOLLERR: Indicates an error condition on the file descriptor.
	*/
	if (!this->addToEpoll(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLERR))
	{
		close(client_fd);
		delete conn;
		this->_connections.erase(client_fd);
		return;
	}

	g_logger.log(LOG_DEBUG, "New connection accepted on fd " + to_string(client_fd));
}

void	ServerManager::handleConnectionEvent(struct epoll_event event)
{
	int	fd = event.data.fd;

	if ((event.events & EPOLLHUP) || (event.events & EPOLLERR))
	{
		g_logger.log(LOG_DEBUG, "Client on fd " + to_string(fd) + " disconnected or error");
		std::map<int, Connection *>::iterator it = this->_connections.find(fd);
		if (it != this->_connections.end())
		{
			epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fd, NULL);

			close(fd);
			delete it->second;
			this->_connections.erase(it);
		}
		return;
	}
}


void	ServerManager::checkTimeouts(void)
{
	std::map<int, Connection *>::iterator it = this->_connections.begin();
	while (it != this->_connections.end())
	{
		Connection *conn = it->second;
		if (conn->isClosed() || conn->isTimeout())
		{
			if (conn->isClosed())
				g_logger.log(LOG_DEBUG, "Connection on fd " + to_string(it->first) + " is closed");
			else
				g_logger.log(LOG_WARNING, "Connection on fd " + to_string(it->first) + " timed out");

			close(it->first);
			delete conn;
			std::map<int, Connection *>::iterator temp = it;
			++it;
			this->_connections.erase(temp);
		}
		else
		{
			++it;
		}
	}
}
