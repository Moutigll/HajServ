/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Port.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 15:11:40 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/03 20:19:18 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PORT_HPP
# define PORT_HPP


# include <netinet/in.h>
# include <fcntl.h>
# include <unistd.h>
# include <cstring>

# include "Logger.hpp"
# include "Config.hpp"

/**
 * @class Port
 * @brief Represents a TCP port with its own socket, and holds multiple servers
 *        that are configured to listen on this port.
 *
 * Each port object is responsible for initializing and managing a listening socket,
 * using non-blocking mode and supporting SO_REUSEADDR.
 * Servers that listen on the same port are stored in a map using their index.
 */
class Port
{
	public:
		Port();

		/**
		 * @brief Constructs a port fd with the specified port number and a server.
		 * @param port_number The TCP port to listen on.
		 * @param srv Pointer to a server instance to associate initially.
		 */
		Port(int port_number, t_server *srv);
		Port(const Port &other);
		Port &operator=(const Port &other);
		~Port();

		/*
		 * @brief Sets up the port socket for listening to incoming client connections.
		 *
		 * This function creates a socket, configures it with necessary options, binds it to
		 * the specified IP address and port, and then starts listening for incoming connections.
		 * It also ensures that the socket is reusable (with SO_REUSEADDR) and handles errors
		 * at each step of the process.
		 * 
		 * @return true if the socket setup was successful, false otherwise.
		*/
		bool	init();

		/**
		 * @brief Adds a server to the list of servers listening on this port.
		 * @param srv A pointer to the server to add.
		 */
		void	addServer(t_server *srv);

		/**
		 * @brief Gets the file descriptor of the listening socket.
		 * @return The socket file descriptor.
		 */
		int		getSocketFd() const;

	private:
		int							_port;
		int							_socket_fd;
		std::vector<t_server *>		_servers;
		std::vector<std::string>	_hosts;

		bool	_handleSocketError(const std::string &message);
};

#endif