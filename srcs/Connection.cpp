/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 22:46:51 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/20 13:24:11 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Connection.hpp"

Connection::Connection(int fd, Server *server, const sockaddr_in &clientAddr)
	: _fd(fd), _server(server), _isKeepAlive(false), _clientAddr(clientAddr)
{
	_lastActivity = std::time(NULL);
	_requestComplete = false;
	_responseBuilt = false;
	_bytesWritten = 0;
	_rawRequest.clear();
	_rawResponse.clear();
}

Connection::~Connection()
{
	if (_fd != -1)
	{
		close(_fd);
		_fd = -1;
	}
	_server->logConnection(_fd, _clientAddr, 0);
}

int	Connection::getFd() const {return (_fd);}

bool	Connection::readRequest()
{
	char		buf[8192];
	ssize_t		bytes_read;
	time_t		now;

	now = std::time(NULL);
	if (now - _lastActivity > static_cast<time_t>(_server->getTimeout()))
		return (false); // Timeout

	bytes_read = recv(_fd, buf, sizeof(buf), 0);
	if (bytes_read < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (true);
		return (false);
	}
	if (bytes_read == 0)
		return (false);

	_lastActivity = now;
	_rawRequest.append(buf, bytes_read);

	if (_rawRequest.find("\r\n\r\n") != std::string::npos)
	{
		_requestComplete = true;
		parseRequest(_rawRequest);
	}
	return (true);
}


bool Connection::isRequestComplete() const
{
	return _requestComplete;
}

bool	Connection::parseRequest(const std::string &raw)
{
	HttpCode	reqStatus;
	
	reqStatus = _request.parseRequest(raw);
	if (g_config.getGlobal("log_requests") == "true")
		_request.logRequest();
	if (reqStatus.getCode() != 200)
	{
		_response.setStatusCode(reqStatus.getCode());
		_response.setHttpVersion(_request.getHttpVersion());
		_response.setHeaders("Content-Type: text/plain; charset=utf-8");
		_response.addHeader("Connection", "close");
		_response.setBody(reqStatus.getMessage(reqStatus.getCode()) + "\n");
		_isKeepAlive = false;
		return (true);
	}

	std::string	connection_header = "";
	if (_request.hasHeader("Connection"))
		connection_header = _request.getHeader("Connection");

	if (_request.getHttpVersion() == "HTTP/1.1")
	{
		if (connection_header == "close")
			_isKeepAlive = false;
		else
			_isKeepAlive = true;
	}
	else if (_request.getHttpVersion() == "HTTP/1.0")
	{
		if (connection_header == "keep-alive")
			_isKeepAlive = true;
		else
			_isKeepAlive = false;
	}
	else
		_isKeepAlive = false;
	return (true);
}

void	Connection::generateResponse()
{
	_response.setStatusCode(200);
	_response.setHttpVersion(_request.getHttpVersion());
	_response.addHeader("Connection", _isKeepAlive ? "keep-alive" : "close");
	if (_request.getMethod() == "GET")
	{
		_server->get(_response, _request.getUri());
	}
	else
	{
		_response.setStatusCode(405);
		_response.setBody("405 Method Not Allowed\n");
	}
}

bool Connection::writeResponse()
{
	time_t	now = std::time(NULL);
	_lastActivity = now;

	if (!_responseBuilt)
	{
		if (_request.isValid())
			generateResponse();
		_rawResponse = _response.serialize();
		_bytesWritten = 0;
		_responseBuilt = true;
	}

	ssize_t n = send(_fd, _rawResponse.c_str() + _bytesWritten, _rawResponse.size() - _bytesWritten, 0);
	if (n < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return true;
		return false;
	}
	_bytesWritten += n;
	if (_bytesWritten < _rawResponse.size())
		return true;

	if (!_isKeepAlive)
		return false;
	return true;
}


bool	Connection::isClosed() const {return (_fd == -1);}

void Connection::resetForNextRequest()
{
	_rawRequest.clear();
	_request = Request();
	_requestComplete = false;
	_responseBuilt = false;
	_rawResponse.clear();
	_bytesWritten = 0;
	_isKeepAlive = false;
}

Server	*Connection::getServer() const
{
	if (_server)
		return _server;
	return NULL;
}