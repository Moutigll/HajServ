/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:52:12 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/03 18:52:50 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Connection.hpp"

Connection::Connection(void)
{
	this->_fd = -1;
	this->_closed = true;
}

Connection::Connection(int fd)
{
	this->_fd = fd;
	this->_closed = false;
}

Connection::Connection(const Connection &other)
{
	this->_fd = other._fd;
	this->_closed = other._closed;
	this->_readBuffer = other._readBuffer;
	this->_writeBuffer = other._writeBuffer;
}

Connection &Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		this->_fd = other._fd;
		this->_closed = other._closed;
		this->_readBuffer = other._readBuffer;
		this->_writeBuffer = other._writeBuffer;
	}
	return (*this);
}

Connection::~Connection(void)
{
	if (this->_fd != -1)
		close(this->_fd);
}

int	Connection::getFd(void) const
{
	return (this->_fd);
}

bool	Connection::isClosed(void) const
{
	return (this->_closed);
}
