/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 22:40:14 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/16 07:31:19 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server() : _server_name("default"),_host("127.0.0.1"), _port(8080), _socket_fd(-1), _timeout(30), _maxBodySize(4200) {}

Server::Server(const std::map<std::string, std::string> &config) : _socket_fd(-1)
{
	std::map<std::string, std::string>::const_iterator it;

	it = config.find("host");
	if (it != config.end())
		this->_host = it->second;
	else
	{
		std::cout << YELLOW << "WARNING: Host not found, defaulting to: 127.0.0.1" << RESET << std::endl;
		this->_host = "127.0.0.1";
	}

	it = config.find("listen");
	if (it != config.end())
		this->_port = std::atoi(it->second.c_str());
	else
	{
		std::cout << YELLOW << "WARNING: Port not found, defaulting to: 8080" << RESET << std::endl;
		this->_port = 8080;
	}

	it = config.find("server_name");
	if (it != config.end())
		this->_server_name = it->second;
	else
	{
		std::cout << YELLOW << "WARNING: Server name not found, defaulting to: default" << RESET << std::endl;
		this->_server_name = "default";
	}
	it = config.find("timeout");
	if (it != config.end())
		this->_timeout = std::atoi(it->second.c_str());
	else
		this->_timeout = 30;
	it = config.find("maxBodySize");
	if (it != config.end())
		this->_maxBodySize = std::atoi(it->second.c_str());
	else
		this->_maxBodySize = 4200;
}

Server::Server(const Server &src)
{
	*this = src;
}

Server &Server::operator=(const Server &src)
{
	if (this != &src)
	{
		this->_host = src._host;
		this->_port = src._port;
		this->_server_name = src._server_name;
		this->_socket_fd = src._socket_fd;
		this->_timeout = src._timeout;
		this->_maxBodySize = src._maxBodySize;
	}
	return *this;
}

Server::~Server()
{
	if (this->_socket_fd != -1)
		close(this->_socket_fd);
}

bool	Server::_handleSocketError(const std::string &message)
{
	std::cerr << RED << message << " for " << this->_server_name << ": "
	          << strerror(errno) << RESET << std::endl;
	if (this->_socket_fd != -1)
	{
		close(this->_socket_fd);
		this->_socket_fd = -1;
	}
	return false;
}


/*
 * @brief Sets up the server socket for listening to incoming client connections.
 *
 * This function creates a socket, configures it with necessary options, binds it to
 * the specified IP address and port, and then starts listening for incoming connections.
 * It also ensures that the socket is reusable (with SO_REUSEADDR) and handles errors
 * at each step of the process.
 * 
 * @return true if the socket setup was successful, false otherwise.
*/
bool	Server::setupSocket()
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

	std::cout << GREEN << "Server [" << this->_server_name << "] listening on " << this->_host << ":" << this->_port << RESET << std::endl;
	return true;
}

void	Server::start()
{
	if (this->_socket_fd == -1)
	{
		if (!this->setupSocket())
			std::cerr << RED << "Failed to set up "<< this->_server_name << RESET << std::endl;
	}
}

const std::string	&Server::getHost() const { return this->_host; }
int					Server::getPort() const { return this->_port; }
const std::string	&Server::getServerName() const { return this->_server_name; }
int					Server::getSocketFd() const { return this->_socket_fd; }
int					Server::getMaxBodySize() const { return this->_maxBodySize; }
int					Server::getTimeout() const { return this->_timeout; }