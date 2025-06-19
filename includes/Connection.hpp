/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:48:52 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/19 18:37:53 by ele-lean         ###   ########.fr       */
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
		bool	isTimeout(void) const;


	private:
		int			_fd;
		Port		*_port;
		t_server	*_server;
		bool		_closed;
		std::time_t	_lastActivity;
};

#endif
