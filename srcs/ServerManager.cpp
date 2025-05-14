/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 23:24:48 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/15 01:13:18 by ele-lean         ###   ########.fr       */
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
}
/**
 * Main server loop using poll().
 * Waits for activity on server sockets (e.g., new connections).
 */
void	ServerManager::runPollLoop()
{
	std::cout << GREEN << "Entering main poll loop..." << RESET << std::endl;

	while (!g_stop)
	{
		int	ret = poll(_pollFds.data(), _pollFds.size(), -1); // Wait indefinitely for events
		if (ret < 0)
		{
			close(g_pipe_fds[0]);
			close(g_pipe_fds[1]);
			if (!g_stop)
				std::cerr << RED << "Poll error: " << strerror(errno) << RESET << std::endl;
			else
				std::cout << GREEN << "Exiting main poll loop..." << RESET << std::endl;
			break;
		}

		for (size_t i = 0; i < _pollFds.size(); ++i)
		{
			if (_pollFds[i].revents & POLLIN)
			{
				int	client_fd = accept(_pollFds[i].fd, NULL, NULL);
				if (client_fd < 0)
				{
					std::cerr << RED << "Accept error: " << strerror(errno) << RESET << std::endl;
					continue;
				}

				std::cout << CYAN << "New connection on socket: " << _pollFds[i].fd << " port: " << _servers[i].getPort() << RESET << std::endl;
				std::string welcome_message = "Hello world from server <" + _servers[i].getServerName() + ">\n";
				send(client_fd, welcome_message.c_str(), welcome_message.size(), 0);
				close(client_fd);
			}
		}
	}
}
