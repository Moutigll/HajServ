/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:48:52 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/20 14:27:34 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Port.hpp"

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


	private:
		int			_fd;
		Port		*_port;
		t_server	*_server;
		bool		_closed;
		std::time_t	_lastActivity;
};

#endif
