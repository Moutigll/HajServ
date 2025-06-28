/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 16:45:25 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/28 10:47:34 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include <sys/epoll.h>

# include "Port.hpp"
# include "Connection.hpp"

#define EPOLL_MAX_EVENTS 256 // Maximum number of events to handle in one epoll_wait call.
#define EPOLL_TIMEOUT 1000 // Timeout in milliseconds for epoll_wait.

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
		void	start(void);

	private:
		int								_epollFd;
		std::vector<Port *>				_ports; // List of listening ports.
		std::map<int, Connection *>		_connections;	// Active client connections.

		/**
		 * @brief Updates the epoll instance with a file descriptor and events.
		 * @param fd File descriptor to monitor.
		 * @param events Events to monitor (EPOLLIN, EPOLLOUT, etc.).
		 * @param op Operation to perform (EPOLL_CTL_ADD, EPOLL_CTL_MOD, etc.).
		 * @return true if the operation succeeded, false on error.
		 */
		bool	updateEpoll(int fd, uint32_t events, int op = EPOLL_CTL_ADD);

		/**
		 * @brief Checks if a file descriptor is a listening socket.
		 * @param fd File descriptor to check.
		 * @return Pointer to Port if fd is a listening socket, NULL otherwise.
		 */
		Port	*isListeningSocket(int fd) const;

		/**
		 * @brief Accepts a new client connection on the given listening port.
		 * 
		 * This function calls accept() on the socket fd of the provided Port object.
		 * If successful, it creates a new Connection object for the client socket,
		 * adds it to the internal connections map, and registers the client fd with epoll.
		 * 
		 * If any error occurs during accept, connection creation, or epoll registration,
		 * the client socket is closed and the Connection object is deleted to avoid leaks.
		 * 
		 * @param port Pointer to the Port object representing the listening socket.
		 *             Must not be NULL and must have a valid socket fd.
		 */
		void	newConnection(Port *port);

		/**
		 * @brief Removes connections that are closed or timed out.
		 */
		void	checkTimeouts(void);
		
		/**
		 * @brief Closes a client connection and cleans up resources.
		 *
		 * This function removes the connection from the internal map of active connections,
		 * closes the associated file descriptor, and deletes the corresponding Connection object.
		 *
		 * @param fd The file descriptor of the connection to close.
		 * @param it An iterator pointing to the connection in the _connections map.
		 */
		void	closeConnection(int fd, std::map<int, Connection *>::iterator it);

		/**
		 * @brief Handles an epoll event for a specific file descriptor.
		 *
		 * This function identifies the associated connection from the file descriptor,
		 * verifies its state, and processes different types of epoll events such as
		 * EPOLLIN (data available to read), EPOLLHUP (hang-up), EPOLLERR (error), and EPOLLRDHUP (peer closed connection).
		 *
		 * @param event The epoll_event structure containing the event details.
		 */
		void	handleConnectionEvent(struct epoll_event event);

		/**
		 * @brief Handles an EPOLLIN event (read readiness) on a client connection.
		 *
		 * This function reads available data from the connection's file descriptor into a buffer,
		 * appends the data to the connection's read buffer, and handles errors or disconnection scenarios.
		 *
		 * @param fd The file descriptor associated with the EPOLLIN event.
		 * @param it An iterator pointing to the connection in the _connections map.
		 */
		void	handleEpollInEvent(int fd, std::map<int, Connection *>::iterator &it);

		/**
		 * @brief Handles an EPOLLOUT event (write readiness) on a client connection.
		 * This function writes data from the connection's write buffer to the file descriptor,
		 * updates the connection state, and handles errors or disconnection scenarios.
		 * @param fd The file descriptor associated with the EPOLLOUT event.
		 * @param it An iterator pointing to the connection in the _connections map. 
		 */
		void	handleEpollOutEvent(int fd, std::map<int, Connection *>::iterator &it);
};

#endif
