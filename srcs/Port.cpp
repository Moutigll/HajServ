/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Port.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 15:39:51 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/03 16:37:54 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Port.hpp"

Port::Port()
: _port(80), _socket_fd(-1), _servers()
{}

Port::Port(int port_number, t_server *srv)
: _port(port_number), _socket_fd(-1), _servers()
{
	if (port_number <= 0 || port_number > 65535)
	{
		g_logger.log(LOG_ERROR, "Invalid port number: " + to_string(port_number));
		_port = 80; // Default to port 80 if invalid
	}
	if (srv)
		_servers.push_back(srv);
}

Port::Port(const Port &other)
: _port(other._port), _socket_fd(other._socket_fd), _servers(other._servers)
{}

Port &Port::operator=(const Port &other)
{
	if (this != &other)
	{
		_port = other._port;
		_socket_fd = other._socket_fd;
		_servers = other._servers;
	}
	return *this;
}

Port::~Port()
{
	if (_socket_fd != -1)
		close(_socket_fd);
}

bool	Port::_handleSocketError(const std::string &message)
{
	g_logger.log(LOG_ERROR, message + " (errno: " + to_string(errno) + ")");
	if (this->_socket_fd != -1)
	{
		close(this->_socket_fd);
		this->_socket_fd = -1;
	}
	return false;
}

bool Port::init()
{
	struct sockaddr_in	server_addr;

	this->_socket_fd = socket(AF_INET, SOCK_STREAM, 0); // Create a File Descriptor for the socket AF_INET->IPv4, SOCK_STREAM->TCP
	if (this->_socket_fd == -1)
		return this->_handleSocketError("Socket creation failed");

	// Set the socket to non-blocking mode
	int	flags = fcntl(this->_socket_fd, F_GETFL, 0);
	if (flags == -1)
		return this->_handleSocketError("fcntl F_GETFL failed");
	if (fcntl(this->_socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return this->_handleSocketError("fcntl F_SETFL O_NONBLOCK failed");
	// Allow the socket to reuse the local address and port immediately after the server shuts down,
	// avoiding the "Address already in use" error when restarting the server quickly.
	int opt = 1;
	if (setsockopt(this->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		return this->_handleSocketError("setsockopt failed");

	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // Set the address family to IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all available interfaces
	server_addr.sin_port = htons(this->_port); // Convert port number to network byte order

	if (bind(this->_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) // Bind the socket to the address and port
		return this->_handleSocketError("Bind failed");

	if (listen(this->_socket_fd, 10) < 0) // Start listening for incoming connections, with a backlog of 10
		return this->_handleSocketError("Listen failed");

	g_logger.log(LOG_INFO, std::string(GREEN) + "Port " + to_string(this->_port) + " is listening on socket fd " + to_string(this->_socket_fd) + RESET);
	return true;
}

void Port::addServer(t_server *srv)
{
	if (srv)
	{
		_servers.push_back(srv);
		g_logger.log(LOG_INFO, "Server added to port " + to_string(_port));
	}
	else
		g_logger.log(LOG_WARNING, "Attempted to add a null server to port " + to_string(_port));
}

int Port::getSocketFd() const
{
	return _socket_fd;
}
