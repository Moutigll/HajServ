/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:52:01 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/21 16:46:11 by ele-lean         ###   ########.fr       */
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

bool	ServerManager::updateEpoll(int fd, uint32_t events, int op)
{
	struct epoll_event	event;

	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(this->_epollFd, op, fd, &event) == -1)
	{
		g_logger.log(LOG_ERROR, "Failed to update fd " + to_string(fd) + " in epoll: " + std::string(strerror(errno)));
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

		if (!this->updateEpoll(port->getSocketFd(), EPOLLIN, EPOLL_CTL_ADD))
		{
			g_logger.log(LOG_ERROR, "Failed to add port " + to_string(port_number) + " to epoll");
			delete port;
			this->_ports.pop_back();
			continue;
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
	if (!port || port->getSocketFd() < 0)
	{
		g_logger.log(LOG_ERROR, "Invalid port for new connection");
		return;
	}

	int	client_fd = accept(port->getSocketFd(), NULL, NULL);
	if (client_fd < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			g_logger.log(LOG_ERROR, "Failed to accept new connection on fd " + to_string(port->getSocketFd()) + ": " + strerror(errno));
		return;
	}

	// Set the client socket to non-blocking mode
	int	flags = fcntl(client_fd, F_GETFL, 0);
	if (flags == -1 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		g_logger.log(LOG_ERROR, "Failed to set non-blocking mode on fd " + to_string(client_fd) + ": " + strerror(errno));
		close(client_fd);
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

	// EPOLLIN = Read events
	// EPOLLET = Edge-triggered mode
	// EPOLLRDHUP = Peer closed connection
	// EPOLLHUP = Hang-up event
	// EPOLLERR = Error event
	if (!this->updateEpoll(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLERR, EPOLL_CTL_ADD))
	{
		g_logger.log(LOG_ERROR, "Failed to add client fd " + to_string(client_fd) + " to epoll: " + std::string(strerror(errno)));
		close(client_fd);
		delete conn;
		this->_connections.erase(client_fd);
		return;
	}

	g_logger.log(LOG_DEBUG, "New connection accepted on fd " + to_string(client_fd));
}


/*------------------
	Events handling
-------------------*/

void	ServerManager::closeConnection(int fd, std::map<int, Connection *>::iterator it)
{
	epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	delete it->second;
	this->_connections.erase(it);
}

void	ServerManager::handleConnectionEvent(struct epoll_event event)
{
	int	fd = event.data.fd;

	std::map<int, Connection *>::iterator it = this->_connections.find(fd);
	if (it == this->_connections.end())
	{
		g_logger.log(LOG_WARNING, "Received event for unknown fd " + to_string(fd));
		epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		return;
	}

	if (it->second->isClosed())
		return;

	it->second->updateLastActivity();

	if ((event.events & EPOLLHUP) || (event.events & EPOLLERR) || (event.events & EPOLLRDHUP))
	{
		if (event.events & EPOLLHUP)
			g_logger.log(LOG_DEBUG, "Client on fd " + to_string(fd) + " hung up, how rude!");
		else if (event.events & EPOLLRDHUP)
			g_logger.log(LOG_DEBUG, "Client on fd " + to_string(fd) + " closed the connection");
		else if (event.events & EPOLLERR)
			g_logger.log(LOG_ERROR, "EPOLLERR event on fd " + to_string(fd) + ": " + std::string(strerror(errno)));
		closeConnection(fd, it);
		return;
	}

	if (event.events & EPOLLIN)
		handleEpollInEvent(fd, it);
	else if (event.events & EPOLLOUT)
		handleEpollOutEvent(fd, it);
	else
		g_logger.log(LOG_WARNING, "Unhandled event on fd " + to_string(fd) + ": " + to_string(event.events));
}

void	ServerManager::handleEpollInEvent(int fd, std::map<int, Connection *>::iterator &it)
{
	char	buffer[4096];
	ssize_t	bytes_read;

	while (1)
	{
		bytes_read = recv(fd, buffer, sizeof(buffer), 0);

		if (bytes_read > 0 && it->second->getState() != WRITING)
			it->second->appendToReadBuffer(buffer, bytes_read);
		else if (bytes_read == 0)
		{
			g_logger.log(LOG_DEBUG, "Connection on fd " + to_string(fd) + " closed by peer");
			closeConnection(fd, it);
			return;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else
			{
				g_logger.log(LOG_ERROR, "Error reading from fd " + to_string(fd) + ": " + std::string(strerror(errno)));
				closeConnection(fd, it);
				return;
			}
		}
	}
	if (it->second->parseRequest() && it->second->getState() == WRITING) // If the request is complete we enable EPOLLOUT
		updateEpoll(fd, EPOLLOUT | EPOLLRDHUP | EPOLLHUP | EPOLLERR, EPOLL_CTL_MOD);
}

void	ServerManager::handleEpollOutEvent(int fd, std::map<int, Connection *>::iterator &it)
{
	if (it->second->getState() != WRITING)
	{
		g_logger.log(LOG_WARNING, "Received EPOLLOUT event on fd " + to_string(fd) + " but connection is not in WRITING state");
		return;
	}
	
	const char *write_buffer = it->second->getReadBuffer();
	if (!write_buffer)
	{
		g_logger.log(LOG_ERROR, "No write buffer available for fd " + to_string(fd));
		updateEpoll(fd, EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR, EPOLL_CTL_MOD);
		return;
	}
	ssize_t	bytes_written = send(fd, write_buffer, strlen(write_buffer), 0);
	if (bytes_written < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			g_logger.log(LOG_DEBUG, "EAGAIN or EWOULDBLOCK on fd " + to_string(fd) + ", will retry later");
			return;
		}
		else
		{
			g_logger.log(LOG_ERROR, "Error writing to fd " + to_string(fd) + ": " + std::string(strerror(errno)));
			closeConnection(fd, it);
			return;
		}
	}
	else if (bytes_written == 0)
	{
		g_logger.log(LOG_DEBUG, "Connection on fd " + to_string(fd) + " closed by peer during write");
		closeConnection(fd, it);
		return;
	}
	it->second->successWrite();
	if (it->second->getState() != WRITING)
	{
		g_logger.log(LOG_DEBUG, "Connection on fd " + to_string(fd) + " finished writing, switching to READING state");
		updateEpoll(fd, EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR, EPOLL_CTL_MOD);
	}
	else
		g_logger.log(LOG_DEBUG, "Partial write on fd " + to_string(fd) + ", " + to_string(bytes_written) + " bytes written, waiting for more data");
	
}


/*--------------------
	Others
---------------------*/

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

			int	fd = it->first;
			epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fd, NULL);
			close(fd);
			delete conn;

			std::map<int, Connection *>::iterator temp = it;
			++it;
			this->_connections.erase(temp);
		}
		else
			++it;
	}
}

