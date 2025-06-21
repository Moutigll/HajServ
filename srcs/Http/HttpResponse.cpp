/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:40:42 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/21 17:07:05 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Http/HttpResponse.hpp"

HttpResponse::HttpResponse(const t_server &server)
	: HttpTransaction(), _server(server), _isHeadersSent(false), _readFd(-1)
{
	_status = 200; // Default status code
	_protocol = "HTTP/1.1";
	_isComplete = false;
}

HttpResponse::HttpResponse(const t_server &server, HttpRequest &request)
	: HttpTransaction(request), _server(server), _isHeadersSent(false), _readFd(-1) {
	_method = request.getMethod();
	_request = request.getRequest();
	_protocol = request.getProtocol();
}

HttpResponse::HttpResponse(const HttpResponse &other)
	: HttpTransaction(other),
	  _server(other._server),
	  _response(other._response),
	  _ErrorStatus(other._ErrorStatus),
	  _filePath(other._filePath),
	  _isHeadersSent(other._isHeadersSent),
	  _readFd(other._readFd) {}

HttpResponse &HttpResponse::operator=(const HttpResponse &other) {
	if (this != &other) {
		HttpTransaction::operator=(other);
		_server = other._server;
		_isHeadersSent = other._isHeadersSent;
		_readFd = other._readFd;
		_response = other._response;
		_ErrorStatus = other._ErrorStatus;
	}
	return *this;
}

HttpResponse::~HttpResponse() {}

bool HttpResponse::isComplete() const {
	return _isComplete;
}

void HttpResponse::setStatus(const HttpError &status) {
	_status = status.getCode();
	_ErrorStatus = status;
}

void HttpResponse::setStatus(int code) {
	_status = code;
}

void HttpResponse::addHeader(const std::string &name, const std::string &value) {
	_headers[name] = value;
}

void HttpResponse::setBody(const std::string &body) {
	_body = body;
}

void HttpResponse::setFilePath(const std::string &filePath) {
	_filePath = filePath;
}

void HttpResponse::construct() {
	_response.clear();
	_response += _protocol + " " + to_string(_status) + " " + _ErrorStatus.getMessage(_status) + "\r\n";
	if (!_body.empty())
		_response += "Content-Length: " + to_string(_body.size()) + "\r\n";
	else
		setFileHeaders();
	std::map<std::string, std::string>::const_iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it) {
		_response += it->first + ": " + it->second + "\r\n";
	}
	_response += "\r\n";
}

HttpError HttpResponse::getStatus() const {
	return _status;
}

char *HttpResponse::sendResponse() {
	if (!_isHeadersSent) {
		if (_response.empty())
			construct();
		if (_response.empty()) {
			g_logger.log(LOG_ERROR, "Failed to construct response: empty response");
			return NULL;
		}
		_isHeadersSent = true;
		char *responseCopy = new char[_response.size() + 1];
		std::memset(responseCopy, 0, _response.size() + 1);
		std::strcpy(responseCopy, _response.c_str());
		return responseCopy;
	}

	char *responseBuffer = new char[_server._max_body_size + 1];
	std::memset(responseBuffer, 0, _server._max_body_size + 1);

	if (_readFd > 0) {
		ssize_t bytesRead = read(_readFd, responseBuffer, _server._max_body_size);
		if (bytesRead < 0) {
			g_logger.log(LOG_ERROR, "Failed to read from file descriptor: " + std::string(strerror(errno)));
			delete[] responseBuffer;
			return NULL;
		}
		if (bytesRead == 0) {
			close(_readFd);
			_readFd = -1;
			_isComplete = true;
			delete[] responseBuffer;
			return NULL;
		}
		responseBuffer[bytesRead] = '\0';
	} else {
		std::strncpy(responseBuffer, _body.c_str(), _server._max_body_size);
		responseBuffer[_server._max_body_size] = '\0';
		_isComplete = true;
	}

	return responseBuffer;
}

void	HttpResponse::setFileHeaders() {
	if (_status >= 400)
	{
		if (_ErrorStatus.getCode() == 200)
			_ErrorStatus.setCode(_status);
		_filePath = _ErrorStatus.getFilePath();
		if (_filePath.empty()) {
			g_logger.log(LOG_ERROR, "No error page defined for status code " + to_string(_status));
			return;
		}
	}
	if (_filePath.empty())
		return;

	struct stat file_stat;
	if (stat(_filePath.c_str(), &file_stat) == 0) {
		_headers["Content-Length"] = to_string(file_stat.st_size);
		_readFd = open(_filePath.c_str(), O_RDONLY);
		if (_readFd < 0) {
			g_logger.log(LOG_ERROR, "Failed to open file: " + _filePath + " - " + strerror(errno));
			setStatus(403);
			return;
		}
	} else {
		setStatus(404);
		return;
	}

	size_t dot = _filePath.find_last_of('.');
	if (dot != std::string::npos) {
		std::string ext = _filePath.substr(dot + 1);
		if (ext == "html" || ext == "htm")
			_headers["Content-Type"] = "text/html";
		else if (ext == "css")
			_headers["Content-Type"] = "text/css";
		else if (ext == "js")
			_headers["Content-Type"] = "application/javascript";
		else if (ext == "png")
			_headers["Content-Type"] = "image/png";
		else if (ext == "jpg" || ext == "jpeg")
			_headers["Content-Type"] = "image/jpeg";
		else if (ext == "gif")
			_headers["Content-Type"] = "image/gif";
		else if (ext == "svg")
			_headers["Content-Type"] = "image/svg+xml";
		else if (ext == "mp4")
			_headers["Content-Type"] = "video/mp4";
		else
			_headers["Content-Type"] = "application/octet-stream";
	} else {
		_headers["Content-Type"] = "application/octet-stream";
	}
}
