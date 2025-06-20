/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:48:52 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/20 22:44:30 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Port.hpp"
# include "Http/HttpRequest.hpp"
# include "Http/HttpResponse.hpp"

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
		 * @brief Append data to the read buffer.
		 *
		 * Allocates the buffer if empty,
		 * otherwise reallocates and appends the new data.
		 *
		 * @param buffer Pointer to the data to append.
		 * @param size Number of bytes to append.
		 * @return true if the operation succeeded, false on allocation failure.
		 */
		bool	appendToReadBuffer(const char *buffer, size_t size);

		/**
		 * @brief Parse the HTTP request from the read buffer.
		 * This function processes the request line, headers, and body
		 * in sequence.
		 * @param buffer The buffer containing the HTTP request data.
		 * @return true if the request was successfully parsed, false if an error occurred or if more data is needed.
		 */
		bool	parseRequest(void);

	private:
		int					_fd;
		Port				*_port;
		e_ConnectionState	_state;
		t_server			*_server;
		bool				_closed;
		char				*_readBuffer;
		std::time_t			_lastActivity;
		HttpTransaction		*_httpTransaction;
		HttpRequest			*_httpRequest;
		HttpResponse		*_httpResponse;
		
		/**
		 * @brief Switch the connection to an error state.
		 * This function sets the connection state to WRITING
		 * and prepares an error response based on the provided error code.
		 * @param errorCode The error code to set for the response.
		 */
		void	switchToErrorState(int errorCode);
	};

#endif
