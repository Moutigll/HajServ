/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:48:52 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/28 11:00:00 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Port.hpp"
# include "Http/HttpRequest.hpp"

enum e_ConnectionState
{
	WRITING,
	READING,
	DONE
};

class Connection
{
	public:
		Connection(void);
		Connection(int fd, Port *port);
		Connection(const Connection &other);
		Connection &operator=(const Connection &other);
		~Connection(void);

		bool	isClosed(void) const;

		/*
		 * @brief Checks if the connection is still active based on the last activity time.
		 * @return true if the connection is still active, false if it has timed out.
		 */
		bool	isTimeout(void) const;
		void	updateLastActivity(void);

		/**
		 * @brief Return the state of the connection.
		 *
		 * The state can be:
		 * - WRITING: The connection is currently writing data.
		 * - READING: The connection is currently reading data.
		 * - DONE: The connection has completed its operation.
		 *
		 * @return The current state of the connection.
		 */
		e_ConnectionState	getState(void) const;

		/**
		 * @brief Parse the HTTP request from the read buffer.
		 * This function processes the request line, headers, and body
		 * in sequence.
		 * @param buffer The buffer containing the HTTP request data.
		 * @return true if the request was successfully parsed, false if an error occurred or if more data is needed.
		 */
		bool	parseRequest(char *buffer);

		/**
		 * @brief Get the write buffer from the HttpResponse object.
		 * @return The write buffer containing the HTTP request data.
		 * @note if the buffer is empty on purpoe its size will be 4242.
		 */
		t_buffer	getWriteBuffer(void);

		/**
		 * @brief Reset the write buffer.
		 * This fuction must be called when a buffer is successfully sent
		 * in order to receave new data.
		 */
		void		successWrite(void);
	private:
		int					_fd; // File descriptor for the client connection
		Port				*_port; // Pointer to the Port object this connection belongs to
		e_ConnectionState	_state; // Current state of the connection (WRITING, READING, DONE)
		t_server			*_server; // Pointer to the server configuration for this connection
		bool				_closed;
		t_buffer			_writeBuffer; // Buffer for writing data to the client
		std::time_t			_lastActivity; // Timestamp of the last activity on this connection
		HttpTransaction		*_httpTransaction; // Poiter to the current HTTP transaction: HttpRequest or HttpResponse
		HttpRequest			*_httpRequest; // The last complete HTTP request received on this connection
		
		/**
		 * @brief Switch the connection to an error state.
		 * This function sets the connection state to WRITING
		 * and prepares an error response based on the provided error code.
		 * @param errorCode The error code to set for the response.
		 */
		void	switchToErrorState(int errorCode);
	};

#endif
