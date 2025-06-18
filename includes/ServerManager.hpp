/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 16:45:25 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/03 20:37:23 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include <sys/epoll.h>

# include "Port.hpp"
# include "Connection.hpp"

/**
 * @class ServerManager
 * @brief Manages multiple listening ports and client connections using epoll.
 *
 * The ServerManager class is responsible for initializing server sockets,
 * monitoring I/O events using epoll, and dispatching events to appropriate
 * handlers (accept new connections, read client data, write responses).
 *
 * Internally, it:
 * - Initializes server ports from configuration.
 * - Creates and manages the epoll file descriptor.
 * - Maintains a list of active connections.
 * - Dispatches I/O events using a non-blocking event loop.
 */
class ServerManager
{
	public:
		ServerManager(void);
		ServerManager(const ServerManager &other);
		ServerManager &operator=(const ServerManager &other);
		~ServerManager(void);

		/**
		 * @brief Initializes ports and epoll.
		 * @param servers Vector of server configurations.
		 * @return true if initialization succeeded.
		 */
		bool	init(const std::vector<t_server> &servers);

		/**
		 * @brief Main server loop using epoll to wait for events.
		 */
		//void	run(void);

	private:
		int								_epollFd;
		std::vector<Port *>				_ports;
		std::map<int, Connection *>		_connections;	// Active client connections.

		/**
		 * @brief Adds a file descriptor to epoll interest list.
		 * @param fd File descriptor to monitor.
		 * @param events Bitmask of events (e.g., EPOLLIN).
		 * @return true on success.
		 */
		bool	addToEpoll(int fd, uint32_t events);

		/**
		 * @brief Handles an epoll event for a specific fd.
		 * @param fd The file descriptor that triggered the event.
		 * @param events The triggered epoll events.
		 */
		void	handleEvent(int fd, uint32_t events);

		/**
		 * @brief Cleans up and closes a connection.
		 * @param fd File descriptor to clean.
		 */
		void	cleanupConnection(int fd);
};

#endif