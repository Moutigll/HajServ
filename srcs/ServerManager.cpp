/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:52:01 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/29 05:55:38 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <csignal>
#include <iostream>

#include "../includes/Logger.hpp"
#include "../includes/ServerManager.hpp"

volatile sig_atomic_t keepRunning = 1;

void signalHandler(int signal)
{
	if (signal == SIGINT || signal == SIGQUIT)
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
	std::map<int, std::vector<t_server const *> >	portToServer; // Map to associate ports with servers

	this->_epollFd = epoll_create1(0);
	if (this->_epollFd == -1)
	{
		g_logger.log(LOG_ERROR, "Failed to create epoll instance: " + std::string(strerror(errno)));
		return false;
	}

	for (std::vector<t_server>::const_iterator it = servers.begin(); it != servers.end(); ++it) // Iterate through all servers
	{
		// For all port in the server add the server to the map at the port key
		for (std::vector<int>::const_iterator pit = it->_ports.begin(); pit != it->_ports.end(); ++pit)
			portToServer[*pit].push_back(&(*it));
	}

	for (std::map<int, std::vector<t_server const *> >::iterator it = portToServer.begin(); it != portToServer.end(); ++it) // Iterate through all ports
	{
		int	port_number = it->first;

		Port *port = new Port(port_number, NULL); // Create a new Port object for the current port number

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

Port	*ServerManager::isListeningSocket(int fd) const
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
	std::signal(SIGQUIT, signalHandler);

	g_logger.log(LOG_INFO, "Server started, waiting for events...");

	while (keepRunning)
	{
		/**
		 * This function stop the code until we recieve an event on the
		 * monitored fd or reach the TIMEOUT treshold wich is used to
		 * forcefully close all connections open for too long
		 */
		nfds = epoll_wait(this->_epollFd, events, EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);
		if (nfds == -1)
		{
			if (errno == EINTR)
				continue;
			g_logger.log(LOG_ERROR, "epoll_wait failed: " + std::string(strerror(errno)));
			break;
		}

		i = 0;
		while (i < nfds) // Iterate through all events received
		{
			int	fd = events[i].data.fd;
			Port *port = isListeningSocket(fd);

			if (port)
				newConnection(port);
			else
				handleConnectionEvent(events[i]);
			++i;
		}
		checkTimeouts();
	}
}

void	ServerManager::newConnection(Port *port)
{
	if (!port || port->getSocketFd() < 0)
	{
		g_logger.log(LOG_ERROR, "Invalid port for new connection");
		return;
	}

	int	client_fd = accept(port->getSocketFd(), NULL, NULL); // Accept a new connection on the listening socket
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

	Connection *conn = new Connection(client_fd, port); // Create a new Connection object for the client
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
	if (!this->updateEpoll(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLERR, EPOLL_CTL_ADD)) // Add the client fd to epoll monitoring
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

	std::map<int, Connection *>::iterator con = this->_connections.find(fd);
	if (con == this->_connections.end()) // If the fd is not found in the connections map, should not happen
	{
		g_logger.log(LOG_WARNING, "Received event for unknown fd " + to_string(fd));
		epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		return;
	}

	if (con->second->isClosed())
		return;

	con->second->updateLastActivity(); // Update the last activity time for the connection to avoid timeout

	if ((event.events & EPOLLHUP) || (event.events & EPOLLERR) || (event.events & EPOLLRDHUP)) // Closing events
	{
		if (event.events & EPOLLHUP)
			g_logger.log(LOG_DEBUG, "Client on fd " + to_string(fd) + " hung up, how rude!");
		else if (event.events & EPOLLRDHUP)
			g_logger.log(LOG_DEBUG, "Client on fd " + to_string(fd) + " closed the connection");
		else if (event.events & EPOLLERR)
			g_logger.log(LOG_ERROR, "EPOLLERR event on fd " + to_string(fd) + ": " + std::string(strerror(errno)));
		closeConnection(fd, con);
		return;
	}

	if (event.events & EPOLLIN)
		handleEpollInEvent(fd, con);
	else if (event.events & EPOLLOUT)
		handleEpollOutEvent(fd, con);
	else
		g_logger.log(LOG_WARNING, "Unhandled event on fd " + to_string(fd) + ": " + to_string(event.events));
}

void ServerManager::handleEpollInEvent(int fd, std::map<int, Connection*>::iterator &it)
{
	bool parseResult = false;
	while (true)
	{
		char buffer[10 + 1]; // +1 for null terminator
		ssize_t bytesRead = recv(fd, buffer, 10, 0);

		if (bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			if (it->second->getState() != WRITING)
				parseResult = it->second->parseRequest(buffer);
		}
		else if (bytesRead == 0)
		{
			// Connection closed by client
			g_logger.log(LOG_DEBUG, "Connection on fd " + to_string(fd) + " closed by peer");
			closeConnection(fd, it);
			return;
		}
		else // bytesRead < 0, stop reading (no errno check possible)
		{
			// Stop reading on error (assuming non-blocking socket)
			break;
		}
	}

	if (parseResult && it->second->getState() == WRITING)
	{
		updateEpoll(fd, EPOLLOUT | EPOLLRDHUP | EPOLLHUP | EPOLLERR, EPOLL_CTL_MOD);
	}
}

void	ServerManager::handleEpollOutEvent(int fd, std::map<int, Connection *>::iterator &con)
{
	if (con->second->getState() != WRITING)
	{
		g_logger.log(LOG_WARNING, "Received EPOLLOUT event on fd " + to_string(fd) + " but connection is not in WRITING state");
		return;
	}
	
	t_buffer writeBuffer = con->second->getWriteBuffer();
	if (!writeBuffer.data && writeBuffer.size == 4242) // 4242 is a magic number indicating no data to write
		return;
	if (!writeBuffer.data || writeBuffer.size == 0)
	{
		g_logger.log(LOG_ERROR, "No write buffer available for fd " + to_string(fd));
		updateEpoll(fd, EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR, EPOLL_CTL_MOD);
		return;
	}
	ssize_t	bytesWritten = send(fd, writeBuffer.data, writeBuffer.size, 0);
	if (bytesWritten < 0)
	{
		g_logger.log(LOG_ERROR, "Error writing to fd " + to_string(fd) + ": " + std::string(strerror(errno)));
		closeConnection(fd, con);
		return;
	} else if (bytesWritten == 0) {
		g_logger.log(LOG_DEBUG, "Connection on fd " + to_string(fd) + " closed by peer during write");
		closeConnection(fd, con);
		return;
	}
	con->second->successWrite();
	if (con->second->getState() == READING)
	{
		g_logger.log(LOG_DEBUG, "Connection on fd " + to_string(fd) + " switched to READING state after writing " + to_string(bytesWritten) + " bytes");
		updateEpoll(fd, EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR, EPOLL_CTL_MOD);
	} else if (con->second->getState() == DONE) {
		g_logger.log(LOG_DEBUG, "Connection on fd " + to_string(fd) + " finished writing " + to_string(bytesWritten) + " bytes, closing connection");
		closeConnection(fd, con);
	}
	else
		g_logger.log(LOG_DEBUG, "Partial write on fd " + to_string(fd) + ", " + to_string(bytesWritten) + " bytes written, waiting for more data");
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
