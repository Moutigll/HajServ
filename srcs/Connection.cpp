/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:52:12 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/26 03:54:51 by etaquet          ###   ########.fr       */
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
      _closed(fd < 0), _lastActivity(std::time(NULL)),
      _httpTransaction(NULL), _httpRequest(NULL), _cgiPid(-1), _cgiPipeFd(-1), _cgiTimeout(0)
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
	{
        switchToErrorState(500);
        return false;
    }

    HttpRequest* request = dynamic_cast<HttpRequest*>(this->_httpTransaction);
    request->setPort(this->_port);
    
    int status = request->parse(readBuffer);
    if (status == -1)
	{
        switchToErrorState(request->getStatus());
        delete request;
        this->_httpTransaction = NULL;
        return true;
    }
    else if (status == 0)
	{
        this->_state = READING;
        return false;
    }

    this->_httpRequest = request;
    this->_httpTransaction = new HttpResponse(*this->_server, *this->_httpRequest, this);
    if (this->_httpTransaction == NULL)
	{
        switchToErrorState(500);
        return false;
    }

    this->_state = WRITING;
    return true;
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


void Connection::setCgiState(pid_t pid, int pipeFd, int timeout) {
    _cgiPid = pid;
    _cgiPipeFd = pipeFd;
    _cgiTimeout = timeout;
    _state = CGI_PROCESSING;
    _cgiStartTime = std::time(NULL);
    std::cout << "CGI start time set to: " << _cgiStartTime << std::endl;
}

void Connection::appendCgiResponse(const char* data, size_t len) {
    _cgiResponse.append(data, len);
}

void Connection::setCgiComplete() {
    if (_cgiPipeFd != -1) {
        close(_cgiPipeFd);
        _cgiPipeFd = -1;
    }
    _state = WRITING;
    _cgiPid = -1;
}

pid_t Connection::getCgiPid() const { return _cgiPid; }

void Connection::setCgiPid( pid_t pid ) { _cgiPid = pid; }
int Connection::getCgiFd() const { return _cgiPipeFd; }
time_t Connection::getCgiStartTime() const { return _cgiStartTime; }
int Connection::getCgiTimeout() const { return _cgiTimeout; }
std::string Connection::getCgiResponse() const { return _cgiResponse; }

void Connection::terminateCgi() {
    if (_cgiPid != -1) {
        // Kill the CGI process and wait for it to ensure no zombies
        kill(_cgiPid, SIGKILL);
        int status;
        waitpid(_cgiPid, &status, 0);
        _cgiPid = -1;
    }
    if (_cgiPipeFd != -1) {
        close(_cgiPipeFd);
        _cgiPipeFd = -1;
    }
    _state = WRITING;
}

void Connection::reapCgi() {
    if (_cgiPid != -1) {
        // Wait for the CGI process to exit normally
        int status;
        waitpid(_cgiPid, &status, 0);
        _cgiPid = -1;
    }
    if (_cgiPipeFd != -1) {
        close(_cgiPipeFd);
        _cgiPipeFd = -1;
    }
    _state = WRITING;
}