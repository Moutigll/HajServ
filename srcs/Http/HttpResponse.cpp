/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:40:42 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/18 19:17:04 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Http/HttpResponse.hpp"

HttpResponse::HttpResponse(const t_server &server)
	: HttpTransaction(), _server(server) {}

HttpResponse::HttpResponse(const HttpResponse &other)
	: HttpTransaction(other), _server(other._server) {}

HttpResponse &HttpResponse::operator=(const HttpResponse &other) {
	if (this != &other) {
		HttpTransaction::operator=(other);
		_server = other._server;
	}
	return *this;
}

HttpResponse::~HttpResponse() {}

bool HttpResponse::isComplete() const {
	return _isComplete;
}

void HttpResponse::setStatus(const HttpError &status) {
	_status = status;
}

void HttpResponse::setStatus(int code) {
	_status = HttpError(code);
}

void HttpResponse::addHeader(const std::string &name, const std::string &value) {
	_headers[name] = value;
}

void HttpResponse::setBody(const std::string &body) {
	_body = body;
}

void HttpResponse::construct() {
	_requestLine = "HTTP/1.1 " + std::to_string(_status.getCode()) + " " + _status.getMessage(_status.getCode());
	_headers["Content-Length"] = std::to_string(_body.size());
}

HttpError HttpResponse::getStatus() const {
	return _status;
}

bool HttpResponse::sendResponse() {
	if (!socketFd) {
		g_logger.log(LOG_ERROR, "Socket not initialized for response sending.");
		return false;
	}
	if (_response.empty()) {
		g_logger.log(LOG_ERROR, "Cannot send empty response.");
		return false;
	}
	ssize_t bytesSent = send(socketFd, _response.c_str(), _response.size(), 0);
	if (bytesSent < 0) {
		g_logger.log(LOG_ERROR, "Failed to send response: " + std::string(strerror(errno)));
		return false;
	}
	if (readFd) {
		char buffer[1024];
		ssize_t bytesRead = read(readFd, buffer, sizeof(buffer) - 1);
		if (bytesRead < 0) {
			g_logger.log(LOG_ERROR, "Failed to read from readFd: " + std::string(strerror(errno)));
			return false;
		}
		if (bytesRead == 0) {
			close(readFd);
			readFd = -1;
			_isComplete = true;
			return true;
		}
		bytesSent = send(socketFd, buffer, bytesRead, 0);
		if (bytesSent < 0)
			g_logger.log(LOG_ERROR, "Failed to send readFd data: " + std::string(strerror(errno)));
		return false;
	}
	_isComplete = true;
	return true;
}
