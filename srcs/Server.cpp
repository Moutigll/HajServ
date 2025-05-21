/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 22:40:14 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/21 12:15:26 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server() : _server_name("default"),_host("127.0.0.1"), _port(8080), _socket_fd(-1), _timeout(30), _maxBodySize(4200) {}

Server::Server(const t_server &server) : _socket_fd(-1)
{
	std::map<std::string, std::string>::const_iterator it;

	it = server._data.find("host");
	if (it != server._data.end())
		this->_host = it->second;
	else
	{
		std::cout << YELLOW << "WARNING: Host not found, defaulting to: 127.0.0.1" << RESET << std::endl;
		this->_host = "127.0.0.1";
	}

	it = server._data.find("listen");
	if (it != server._data.end())
		this->_port = std::atoi(it->second.c_str());
	else
	{
		std::cout << YELLOW << "WARNING: Port not found, defaulting to: 8080" << RESET << std::endl;
		this->_port = 8080;
	}

	it = server._data.find("server_name");
	if (it != server._data.end())
		this->_server_name = it->second;
	else
	{
		std::cout << YELLOW << "WARNING: Server name not found, defaulting to: default" << RESET << std::endl;
		this->_server_name = "default";
	}

	it = server._data.find("timeout");
	if (it != server._data.end())
		this->_timeout = std::atoi(it->second.c_str());
	else
		this->_timeout = 30;

	it = server._data.find("maxBodySize");
	if (it != server._data.end())
		this->_maxBodySize = std::atoi(it->second.c_str());
	else
		this->_maxBodySize = 4200;

	this->_locations = server._locations;
	if (this->_locations.empty())
	{
		std::cout << YELLOW << "WARNING: No locations found, defaulting to: /" << RESET << std::endl;
		this->_locations.push_back(Location());
		this->_locations.back().setLocation("/");
		this->_locations.back().setRoot("./");
	}
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
		this->_locations = src._locations;
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

void Server::logConnection(int client_fd, const sockaddr_in &client_addr, int mode)
{
	if (g_config.getGlobal("log_connections") != "true")
		return;
	char client_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

	std::cout << YELLOW << getTimestamp();

	if (mode == 1)
		std::cout << GREEN << " ➡️  New Connection from "; 
	else
		std::cout << RED << " ⬅️  Connection Closed from ";

	std::cout << client_ip << ":" << ntohs(client_addr.sin_port)
			  << " on server [" << this->_server_name << ":" << this->_port << "]"
			  << " | fd=" << client_fd
			  << RESET << std::endl;
}

const Location	*Server::matchLocation(const std::string &uri) const
{
	std::vector<Location>::const_iterator	it;
	std::string								longest_match;
	const Location							*best_match = NULL;

	for (it = this->_locations.begin(); it != this->_locations.end(); ++it)
	{
		const std::string	&location_path = it->getLocation();
		if (uri.find(location_path) == 0 && location_path.length() > longest_match.length())
		{
			longest_match = location_path;
			best_match = &(*it);
		}
	}
	return best_match;
}

void	Server::get(Response &response, const std::string &uri)
{
	const Location	*location = this->matchLocation(uri);

	if (location)
	{
		std::string filepath = location->getRoot() + uri.substr(location->getLocation().length());

		std::ifstream	file(filepath.c_str());
		if (!file.is_open())
		{
			response.setStatusCode(404);
			response.setBody("404 Not Found");
			return;
		}

		std::stringstream	buffer;
		buffer << file.rdbuf();
		file.close();

		response.setStatusCode(200);
		response.setBody(buffer.str());
	}
	else
	{
		std::cout << "No matching location found" << std::endl;
		response.setStatusCode(404);
		response.setBody("404 Not Found");
	}
}
