/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moutig <moutig@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:16:25 by ele-lean          #+#    #+#             */
/*   Updated: 2025/07/02 21:19:02 by moutig           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/HttpTransaction.hpp"
#include "../../includes/Logger.hpp"

HttpTransaction::HttpTransaction()
	: _connectionKeepAlive(false), _status(200), _isComplete(false) {}

HttpTransaction::HttpTransaction(const HttpTransaction &other)
	: _method(other._method),
	  _uri(other._uri),
	  _query(other._query),
	  _connectionKeepAlive(other._connectionKeepAlive),
	  _protocol(other._protocol),
	  _headers(other._headers),
	  _body(other._body),
	  _status(other._status),
	  _isComplete(other._isComplete) {}

HttpTransaction &HttpTransaction::operator=(const HttpTransaction &other) {
	if (this != &other) {
		_uri = other._uri;
		_query = other._query;
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
		"Request Line:\n\t" + _method + " " + _uri + " " + _protocol + "\n"
		"Headers:\n";

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		message += "\t" + it->first + ": " + it->second + "\n";
	}

	message += "Body:\n\t[" + _body + "]\n";

	g_logger.log(LOG_DEBUG, message);
}


/*-------------------------
	Getters and Setters
---------------------------*/

int HttpTransaction::getStatus() const {
	return _status;
}

std::string HttpTransaction::getMethod() const {
	return _method;
}

std::string HttpTransaction::getRequest() const {
	return _uri;
}

std::string HttpTransaction::getQuery() const {
	return _query;
}

std::string HttpTransaction::getProtocol() const {
	return _protocol;
}

bool HttpTransaction::isConnectionKeepAlive() const {
	return _connectionKeepAlive;
}

std::string HttpTransaction::getBody() const {
	return _body;
}

std::map<std::string, std::string> HttpTransaction::getHeaders() const {
	return _headers;
}

t_server *HttpTransaction::getServer() const {
	return _server;
}
