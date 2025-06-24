/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:16:25 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/24 02:34:41 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Http/HttpTransaction.hpp"

HttpTransaction::HttpTransaction()
	: _connectionKeepAlive(false), _status(200), _isComplete(false) {}

HttpTransaction::HttpTransaction(const HttpTransaction &other)
	: _method(other._method),
	  _request(other._request),
	  _connectionKeepAlive(other._connectionKeepAlive),
	  _protocol(other._protocol),
	  _headers(other._headers),
	  _body(other._body),
	  _status(other._status),
	  _isComplete(other._isComplete) {}

HttpTransaction &HttpTransaction::operator=(const HttpTransaction &other) {
	if (this != &other) {
		_request = other._request;
		_method = other._method;
		_connectionKeepAlive = other._connectionKeepAlive;
		_protocol = other._protocol;
		_headers = other._headers;
		_body = other._body;
		_status = other._status;
		_isComplete = other._isComplete;
	}
	return *this;
}

HttpTransaction::~HttpTransaction() {}

void HttpTransaction::log() const {
	std::string message = "HTTP Transaction:\n"
		"Request Line:\n\t" + _method + " " + _request + " " + _protocol + "\n"
		"Headers:\n";

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		message += "\t" + it->first + ": " + it->second + "\n";
	}

	message += "Body:\n\t[" + _body + "]\n";

	g_logger.log(LOG_DEBUG, message);
}

int HttpTransaction::getStatus() const {
	return _status;
}

std::string HttpTransaction::getMethod() const {
	return _method;
}

std::string HttpTransaction::getRequest() const {
	return _request;
}

std::string HttpTransaction::getProtocol() const {
	return _protocol;
}

bool HttpTransaction::isConnectionKeepAlive() const {
	return _connectionKeepAlive;
}
