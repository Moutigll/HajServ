/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:52:12 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/24 03:30:39 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Connection.hpp"

Connection::Connection(void)
	: _fd(-1), _port(NULL), _state(READING), _server(NULL),
	  _closed(true), _writeBuffer(NULL), _lastActivity(std::time(NULL)), _httpTransaction(NULL), _httpRequest(NULL) {}

Connection::Connection(int fd, Port *port)
	: _fd(fd), _port(port), _state(READING), _server(NULL),
	  _closed(fd < 0), _writeBuffer(NULL), _lastActivity(std::time(NULL)), _httpTransaction(NULL), _httpRequest(NULL)
{
	if (port != NULL)
	{
		this->_server = port->getServer(0);
		if (this->_server == NULL)
			this->_closed = true;
	}
	else
		this->_closed = true;
}

Connection::Connection(const Connection &other)
	: _fd(other._fd), _port(other._port), _state(other._state),
	  _server(other._server), _closed(other._closed), _writeBuffer(NULL),
	  _lastActivity(other._lastActivity), _httpTransaction(other._httpTransaction), _httpRequest(other._httpRequest) {}


Connection &Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		this->_fd = other._fd;
		this->_closed = other._closed;
		this->_port = other._port;
		this->_lastActivity = other._lastActivity;
		this->_server = other._server;
		this->_state = other._state;
		this->_httpTransaction = other._httpTransaction;
		this->_httpRequest = other._httpRequest;
	}
	return *this;
}

Connection::~Connection(void)
{
	if (_httpRequest != NULL)
	{
		delete _httpRequest;
		_httpRequest = NULL;
	}
	if (_httpTransaction != NULL)
	{
		delete _httpTransaction;
		_httpTransaction = NULL;
	}
	if (_writeBuffer != NULL)
	{
		delete[] _writeBuffer;
		_writeBuffer = NULL;
	}
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

void	Connection::updateLastActivity(void)
{
	this->_lastActivity = std::time(NULL);
}

e_ConnectionState	Connection::getState(void) const
{
	return (this->_state);
}

void	Connection::switchToErrorState(int errorCode)
{
	g_logger.log(LOG_DEBUG, "Switching to error state with code: " + to_string(errorCode) + " on fd: " + to_string(this->_fd));
	this->_httpTransaction = NULL;
	this->_httpTransaction = new HttpResponse(*this->_server);
	if (this->_httpTransaction == NULL)
		return;
	HttpResponse* response = dynamic_cast<HttpResponse*>(this->_httpTransaction);
	response->setStatus(errorCode);
	this->_state = WRITING;
}

bool Connection::parseRequest(char *readBuffer)
{
	if (this->_httpRequest != NULL)
	{
		delete this->_httpRequest;
		this->_httpRequest = NULL;
	}
	
	if (this->_httpTransaction == NULL)
		this->_httpTransaction = new HttpRequest();
	if (this->_httpTransaction == NULL)
		return (switchToErrorState(500), false); // Allocation failed, switch to error state 500 internal server error

	HttpRequest* request = dynamic_cast<HttpRequest*>(this->_httpTransaction);
	request->setPort(this->_port);
	
	int status = request->parse(readBuffer);
	if (status == -1)
	{
		switchToErrorState(request->getStatus());
		delete request;
		return true;
	}
	else if (status == 0)
	{
		this->_state = READING; // Need more data
		return false;
	}
	this->_httpRequest = request;
	this->_httpTransaction = NULL;
	_httpRequest->log();
	this->_state = WRITING;
	return true; // Request is complete, switch to writing state
}

char *Connection::getReadBuffer(void)
{
	if (this->_writeBuffer != NULL)
		return this->_writeBuffer;
	if (this->_httpTransaction == NULL)
	{
		if (this->_httpRequest != NULL)
			this->_httpTransaction = new HttpResponse(*this->_server, *this->_httpRequest);
		else
			this->_httpTransaction = new HttpResponse(*this->_server);
	}
	if (this->_httpTransaction == NULL)
		return NULL;
	HttpResponse* response = dynamic_cast<HttpResponse*>(this->_httpTransaction);
	_writeBuffer = response->sendResponse();
	if (response->isComplete())
	{
		delete this->_httpTransaction;
		this->_httpTransaction = NULL;
		this->_state = DONE; // Request is complete, switch to done state
	}
	return this->_writeBuffer;
}

void Connection::successWrite(void)
{
	delete[] this->_writeBuffer;
	this->_writeBuffer = NULL; // Clear write buffer after successful write
}
