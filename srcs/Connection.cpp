/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:52:12 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/19 19:21:21 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Connection.hpp"

Connection::Connection(void)
{
	this->_fd = -1;
	this->_closed = true;
	this->_port = NULL;
	this->_lastActivity = std::time(NULL);
	this->_server = NULL;
}

Connection::Connection(int fd, Port *port) : _fd(fd), _port(port)
{
	if (fd < 0)
	{
		this->_fd = -1;
		this->_closed = true;
	}
	else
		this->_closed = false;
	this->_lastActivity = std::time(NULL);
	if (port != NULL)
	{
		this->_server = port->getServer(0);
		if (this->_server == NULL)
			this->_closed = true;
	}
	else
	{
		this->_server = NULL;
		this->_closed = true;
	}
}

Connection::Connection(const Connection &other)
{
	this->_fd = other._fd;
	this->_port = other._port;
	this->_closed = other._closed;
	this->_lastActivity = other._lastActivity;
	this->_server = other._server;
}

Connection &Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		this->_fd = other._fd;
		this->_port = other._port;
		this->_closed = other._closed;
		this->_lastActivity = other._lastActivity;
		this->_server = other._server;
	}
	return (*this);
}

Connection::~Connection(void)
{
	if (this->_fd != -1)
		close(this->_fd);
}

bool	Connection::isClosed(void) const
{
	return (this->_closed);
}

bool	Connection::isTimeout(void) const
{
	if (this->_server == NULL)
		return true;
	std::time_t now = std::time(NULL);
	return (difftime(now, this->_lastActivity) > this->_server->_timeout);
}
