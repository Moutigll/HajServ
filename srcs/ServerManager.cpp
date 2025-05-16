/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 23:24:48 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/16 07:45:30 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerManager.hpp"

ServerManager::ServerManager() {}

ServerManager::ServerManager(const ServerManager &src)
{
	*this = src;
}

ServerManager &ServerManager::operator=(const ServerManager &src)
{
	if (this != &src)
		this->_servers = src._servers;
	return *this;
}

ServerManager::~ServerManager() {}

void	ServerManager::addServer(const std::map<std::string, std::string> &config)
{
	Server server(config);
	this->_servers.push_back(server);
}

Server*	ServerManager::findServerByFd(int fd)
{
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		if (_servers[i].getSocketFd() == fd)
			return &_servers[i];
	}
	return NULL;
}

bool	ServerManager::isListeningSocket(int fd) const
{
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		if (_servers[i].getSocketFd() == fd)
			return true;
	}
	return false;
}

void	ServerManager::cleanupClosedConnections()
{
	std::vector<int>	to_remove;

	for (std::map<int, Connection*>::iterator it = _connections.begin(); it != _connections.end(); ++it)
	{
		if (it->second && it->second->isClosed())
			to_remove.push_back(it->first);
	}

	for (size_t i = 0; i < to_remove.size(); ++i)
	{
		int	fd = to_remove[i];
		close(fd);
		for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
		{
			if (it->fd == fd)
			{
				_pollFds.erase(it);
				break;
			}
		}
		delete _connections[fd];
		_connections.erase(fd);
		std::cout << RED << "Closed connection: " << fd << RESET << std::endl;
	}
}

void	ServerManager::cleanup() {
	for (std::map<int, Connection*>::iterator it = this->_connections.begin(); it != this->_connections.end(); ++it) {
		close(it->first);
		std::cout << RED << "Closed connection: " << it->first << RESET << std::endl;
		delete it->second;
	}
	this->_connections.clear();

	for (size_t i = 0; i < this->_servers.size(); i++) {
		close(this->_servers[i].getSocketFd());
	}
}

/**
 * Starts all servers and prepares them to accept connections.
 *
 * For each server, this sets up the socket and adds it to a list of sockets to monitor.
 * The poll() system call will later check these sockets for activity (like new connections).
 * 
 * We use POLLIN to watch for read events — for server sockets, this means a client is trying to connect.
 * Once all sockets are added, runPollLoop() will start the main loop using poll().
 */
void	ServerManager::startServers()
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		if (!_servers[i].setupSocket())
		{
			std::cerr << RED << "Failed to set up server " << _servers[i].getServerName() << " on "
				<< _servers[i].getHost() << ":" << _servers[i].getPort() << RESET << std::endl;
			continue;
		}

		struct pollfd pfd;
		pfd.fd = _servers[i].getSocketFd();
		pfd.events = POLLIN;
		_pollFds.push_back(pfd);
	}
	struct pollfd pfd;
	pfd.fd = g_pipe_fds[0];
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollFds.push_back(pfd); // Add the read end of the pipe to pollFds to handle signals
	runPollLoop();
	close(g_pipe_fds[0]);
	close(g_pipe_fds[1]);
	cleanup();
}

/**
 * Main server loop using poll().
 * Waits for activity on server sockets (e.g., new connections).
 * Handles incoming connections, reads requests, and sends responses.
 * Also checks for timeouts and cleans up closed connections.
 */
void ServerManager::runPollLoop()
{
	std::cout << GREEN << "Entering main poll loop..." << RESET << std::endl;
	const int pollTimeoutMs = 1000;

	while (!g_stop)
	{
		int ret = poll(_pollFds.data(), _pollFds.size(), pollTimeoutMs);
		if (ret < 0)
		{
			if (!g_stop)
				std::cerr << RED << "Poll error: " << strerror(errno) << RESET << std::endl;
			else
				std::cout << GREEN << "Exiting main poll loop..." << RESET << std::endl;
			break;
		}
		
		for (size_t i = 0; i < _pollFds.size(); ++i)
		{
			int fd = _pollFds[i].fd;
			if (isListeningSocket(fd) && (_pollFds[i].revents & POLLIN))
			{
				acceptNewConnections(fd);
			}
		}

		handleIoEvents();
		checkTimeouts();
		cleanupClosedConnections();
	}
}

/*
 * Accepts new client connections on the listening socket.
 * For each new connection, creates a Connection object and adds it to the list of connections.
 * Also sets the socket to non-blocking mode.
 */
void ServerManager::acceptNewConnections(int listenFd)
{
	while (true)
	{
		int clientFd = accept(listenFd, NULL, NULL);
		if (clientFd < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			std::cerr << RED << "Accept error: " << strerror(errno) << RESET << std::endl;
			break;
		}
		// passe en non-bloquant
		int flags = fcntl(clientFd, F_GETFL, 0);
		fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

		Server* srv = findServerByFd(listenFd);
		if (!srv)
		{
			std::cerr << RED << "No server for fd " << listenFd << RESET << std::endl;
			close(clientFd);
			continue;
		}

		Connection* conn = new Connection(clientFd, srv);
		_connections[clientFd] = conn;

		struct pollfd pfd = { clientFd, POLLIN, 0 };
		_pollFds.push_back(pfd);

		std::cout << CYAN << "Accepted new connection: " << clientFd << RESET << std::endl;
	}
}

/*
 * Handles I/O events for each connection.
 * Reads requests from clients and sends responses.
 * If a connection is closed or an error occurs, it cleans up the connection.
 */
void ServerManager::handleIoEvents()
{
	for (size_t i = 0; i < _pollFds.size(); ++i)
	{
		int fd = _pollFds[i].fd;
		if (isListeningSocket(fd)) continue;

		Connection* conn = _connections[fd];

		if ((_pollFds[i].revents & POLLIN) && !conn->isRequestComplete())
		{
			if (!conn->readRequest())
			{
				markConnectionClosed(fd, i);
				--i;
				continue;
			}
			if (conn->isRequestComplete())
			{
				_pollFds[i].events = POLLOUT;
			}
		}
		else if ((_pollFds[i].revents & POLLOUT) && conn->isRequestComplete())
		{
			bool keepAlive = conn->writeResponse();
			if (!keepAlive)
			{
				markConnectionClosed(fd, i);
				--i;
				continue;
			}
			else
			{
				conn->resetForNextRequest();
				_pollFds[i].events = POLLIN;
			}
		}
	}
}

void ServerManager::markConnectionClosed(int fd, size_t pollIndex)
{
	close(fd);
	delete _connections[fd];
	_connections.erase(fd);
	_pollFds.erase(_pollFds.begin() + pollIndex);
	std::cout << RED << "Closed connection: " << fd << RESET << std::endl;
}

/*
 * Checks for timeouts on each connection.
 * If a connection has been inactive for too long, it is closed.
 * The duration is defined for each server, 30 seconds by default.
 */
void ServerManager::checkTimeouts()
{
	time_t now = std::time(NULL);
	std::vector<int> toClose;

	for (std::map<int, Connection*>::const_iterator it = _connections.begin(); it != _connections.end(); ++it)
	{
		int fd = it->first;

		if (it->second == NULL)
			continue;
		Connection* conn = it->second;
		Server* srv = conn->getServer();

		int timeout = srv->getTimeout();
		if (timeout == 0)
			continue;

		if (timeout > 0 && (now - conn->getLastActivity() >= timeout))
			toClose.push_back(fd);

	}

	for (std::vector<int>::iterator it = toClose.begin(); it != toClose.end(); ++it)
	{
		int fd = *it;
		for (size_t i = 0; i < _pollFds.size(); ++i)
		{
			if (_pollFds[i].fd == fd)
			{
				markConnectionClosed(fd, i);
				break;
			}
		}
	}
}
