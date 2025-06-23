/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:52:12 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/23 17:19:53 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Connection.hpp"

Connection::Connection(void)
	: _fd(-1), _port(NULL), _state(READING), _server(NULL),
	  _closed(true), _readBuffer(NULL), _writeBuffer(NULL), _lastActivity(std::time(NULL)), _httpTransaction(NULL), _httpRequest(NULL), _httpResponse(NULL) {}

Connection::Connection(int fd, Port *port)
	: _fd(fd), _port(port), _state(READING), _server(NULL),
	  _closed(fd < 0), _readBuffer(NULL), _writeBuffer(NULL), _lastActivity(std::time(NULL)), _httpTransaction(NULL), _httpRequest(NULL), _httpResponse(NULL)
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
	  _server(other._server), _closed(other._closed), _readBuffer(NULL), _writeBuffer(NULL),
	  _lastActivity(other._lastActivity), _httpTransaction(other._httpTransaction), _httpRequest(other._httpRequest), _httpResponse(other._httpResponse)
{
	if (other._readBuffer != NULL)
	{
		size_t len = strlen(other._readBuffer);
		this->_readBuffer = new char[len + 1];
		if (this->_readBuffer != NULL)
			memcpy(this->_readBuffer, other._readBuffer, len + 1);
	}
}


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
		this->_httpResponse = other._httpResponse;

		delete[] this->_readBuffer;
		this->_readBuffer = NULL;
		if (other._readBuffer != NULL)
		{
			size_t len = strlen(other._readBuffer);
			this->_readBuffer = new char[len + 1];
			if (this->_readBuffer != NULL)
				memcpy(this->_readBuffer, other._readBuffer, len + 1);
		}
	}
	return *this;
}

Connection::~Connection(void)
{
	delete[] this->_readBuffer;
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

bool	Connection::appendToReadBuffer(const char *buffer, size_t size)
{
	if (this->_readBuffer == NULL)
	{
		this->_readBuffer = new char[size + 1];
		if (this->_readBuffer == NULL)
			return false;
		memcpy(this->_readBuffer, buffer, size);
		this->_readBuffer[size] = '\0';
	}
	else
	{
		size_t old_len = strlen(this->_readBuffer);
		char *newBuffer = new char[old_len + size + 1];
		if (newBuffer == NULL)
			return false;
		memcpy(newBuffer, this->_readBuffer, old_len);
		memcpy(newBuffer + old_len, buffer, size);
		newBuffer[old_len + size] = '\0';
		delete[] this->_readBuffer;
		this->_readBuffer = newBuffer;
	}
	return true;
}

void	Connection::switchToErrorState(int errorCode)
{
	g_logger.log(LOG_DEBUG, "Switching to error state with code: " + to_string(errorCode) + " on fd: " + to_string(this->_fd));
	if (this->_httpResponse != NULL)
		delete this->_httpResponse;
	this->_httpTransaction = NULL;
	this->_httpResponse = new HttpResponse(*this->_server);
	if (this->_httpResponse == NULL)
		return;
	_httpTransaction = _httpResponse;
	this->_httpResponse->setStatus(errorCode);
	this->_state = WRITING;
}

bool Connection::parseRequest(void)
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
	if (request == NULL)
		return (switchToErrorState(500), false); // Dynamic cast failed, should not happen
	
	int status = request->parse(this->_readBuffer);
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
		this->_httpTransaction = new HttpResponse(*this->_server);
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
