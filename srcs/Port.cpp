/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Port.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 15:39:51 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/28 10:49:35 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Colors.hpp"
#include "../includes/Logger.hpp"
#include "../includes/Port.hpp"

Port::Port()
: _port(80), _socketFd(-1), _servers(), _hosts()
{}

Port::Port(int port_number, t_server *srv)
: _port(port_number), _socketFd(-1), _servers(), _hosts()
{
	if (port_number <= 0 || port_number > 65535)
	{
		g_logger.log(LOG_ERROR, "Invalid port number: " + to_string(port_number));
		_port = 80; // Default to port 80 if invalid
	}
	if (srv)
	{
		_servers.push_back(srv);
		_hosts = srv->_hosts;
	}
}

Port::Port(const Port &other)
: _port(other._port), _socketFd(other._socketFd), _servers(other._servers)
{}

Port &Port::operator=(const Port &other)
{
	if (this != &other)
	{
		_port = other._port;
		_socketFd = other._socketFd;
		_servers = other._servers;
	}
	return *this;
}

Port::~Port()
{
	if (_socketFd != -1)
		close(_socketFd);
}

bool	Port::_handleSocketError(const std::string &message)
{
	g_logger.log(LOG_ERROR, message + " (errno: " + strerror(errno) + ")");
	if (this->_socketFd != -1)
	{
		close(this->_socketFd);
		this->_socketFd = -1;
	}
	return false;
}

bool Port::init()
{
	struct sockaddr_in	server_addr;

	this->_socketFd = socket(AF_INET, SOCK_STREAM, 0); // Create a File Descriptor for the socket AF_INET->IPv4, SOCK_STREAM->TCP
	if (this->_socketFd == -1)
		return this->_handleSocketError("Socket creation failed");

	// Set the socket to non-blocking mode
	int	flags = fcntl(this->_socketFd, F_GETFL, 0);
	if (flags == -1)
		return this->_handleSocketError("fcntl F_GETFL failed");
	if (fcntl(this->_socketFd, F_SETFL, flags | O_NONBLOCK) == -1)
		return this->_handleSocketError("fcntl F_SETFL O_NONBLOCK failed");
	// Allow the socket to reuse the local address and port immediately after the server shuts down,
	// avoiding the "Address already in use" error when restarting the server quickly.
	int opt = 1;
	if (setsockopt(this->_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		return this->_handleSocketError("setsockopt failed");

	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // Set the address family to IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all available interfaces
	server_addr.sin_port = htons(this->_port); // Convert port number to network byte order

	if (bind(this->_socketFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) // Bind the socket to the address and port
		return this->_handleSocketError("Bind failed");

	if (listen(this->_socketFd, 10) < 0) // Start listening for incoming connections, with a backlog of 10
		return this->_handleSocketError("Listen failed");

	std::string hosts_list;
	for (std::vector<std::string>::const_iterator it = _hosts.begin(); it != _hosts.end(); ++it)
	{
		hosts_list += *it;
		if (it + 1 != _hosts.end())
			hosts_list += ", ";
	}

	g_logger.log(LOG_INFO, std::string(GREEN) + "Port " + to_string(this->_port) +
		" is listening on socket fd " + to_string(this->_socketFd) + RESET);
	g_logger.log(LOG_DEBUG, "Hosts listening on port " + to_string(this->_port) + ": " + hosts_list);

	
	return true;
}

void Port::addServer(t_server *srv)
{
	if (srv)
	{
		_servers.push_back(srv);
		if (srv->_hosts.empty())
			g_logger.log(LOG_WARNING, "Server added to port " + to_string(_port) + " has no hosts specified.");
		else
			_hosts.insert(_hosts.end(), srv->_hosts.begin(), srv->_hosts.end());
	}
	else
		g_logger.log(LOG_WARNING, "Attempted to add a null server to port " + to_string(_port));
}

int Port::getSocketFd() const
{
	return _socketFd;
}

t_server *Port::getServer(size_t index) const
{
	if (index < _servers.size())
		return _servers[index];
	return NULL;
}
