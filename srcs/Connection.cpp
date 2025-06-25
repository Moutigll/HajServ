/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:52:12 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/25 04:57:26 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Connection.hpp"
#include "../includes/Http/HttpResponse.hpp"

Connection::Connection(void)
	: _fd(-1), _port(NULL), _state(READING), _server(NULL),
	  _closed(true), _lastActivity(std::time(NULL)), _httpTransaction(NULL), _httpRequest(NULL)
{
	_writeBuffer.data = NULL;
	_writeBuffer.size = 0;
}

Connection::Connection(int fd, Port *port)
	: _fd(fd), _port(port), _state(READING), _server(NULL),
	  _closed(fd < 0), _lastActivity(std::time(NULL)), _httpTransaction(NULL), _httpRequest(NULL)
{
	if (port != NULL)
	{
		this->_server = port->getServer(0);
		if (this->_server == NULL)
			this->_closed = true;
	}
	else
		this->_closed = true;
	_writeBuffer.data = NULL;
	_writeBuffer.size = 0;
}

Connection::Connection(const Connection &other)
	: _fd(other._fd), _port(other._port), _state(other._state),
	  _server(other._server), _closed(other._closed),
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
	if (_writeBuffer.data != NULL)
	{
		delete[] _writeBuffer.data;
		_writeBuffer.data = NULL;
		_writeBuffer.size = 0;
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
	g_logger.log(LOG_DEBUG, "Request parsed successfully on fd: " + to_string(this->_fd));
	_httpRequest->log();
	this->_state = WRITING;
	return true; // Request is complete, switch to writing state
}

t_buffer	Connection::getReadBuffer(void)
{
	if (this->_writeBuffer.data != NULL)
		return this->_writeBuffer;
	if (this->_httpTransaction == NULL)
	{
		if (this->_httpRequest != NULL)
			this->_httpTransaction = new HttpResponse(*this->_server, *this->_httpRequest);
		else
			this->_httpTransaction = new HttpResponse(*this->_server);
	}
	if (this->_httpTransaction == NULL)
	{
		t_buffer buf;
		buf.data = NULL;
		buf.size = 0;
		return buf;
	}
	HttpResponse* response = dynamic_cast<HttpResponse*>(this->_httpTransaction);
	_writeBuffer = response->sendResponse();
	if (response->isComplete())
	{
		if (this->_httpTransaction->isConnectionKeepAlive())
			this->_state = READING; // Keep connection alive, switch to reading state
		else
			this->_state = DONE; // Close connection after response
		delete this->_httpTransaction;
		this->_httpTransaction = NULL;
	}
	return this->_writeBuffer;
}

void Connection::successWrite(void)
{
	// Clear write buffer after successful write
	if (this->_writeBuffer.data != NULL)
	{
		delete[] this->_writeBuffer.data;
		this->_writeBuffer.data = NULL;
		this->_writeBuffer.size = 0;
	}
}
