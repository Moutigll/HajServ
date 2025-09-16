/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 21:05:28 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/29 06:41:21 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/HttpRequest.hpp"
# include <algorithm>
#include <iostream>

HttpRequest::HttpRequest(Port *port)
	: HttpTransaction(),
	  _parseState(PS_REQUEST_LINE),
	  _contentLenght(0),
	  _port(port)
{
	_isComplete = false;
}

HttpRequest::HttpRequest(const HttpRequest &other)
	: HttpTransaction(other),
	  _parseState(other._parseState),
	  _accum(other._accum),
	  _contentLenght(other._contentLenght)
{
}

HttpRequest &HttpRequest::operator=(const HttpRequest &other) {
	if (this != &other) {
		HttpTransaction::operator=(other);
		_parseState   = other._parseState;
		_accum         = other._accum;
		_contentLenght= other._contentLenght;
	}
	return *this;
}

HttpRequest::~HttpRequest() {}



int HttpRequest::parse(const char *buffer)
{
	if (_parseState == PS_ERROR || _parseState == PS_DONE)
		return _parseState == PS_DONE ? 1 : -1;

	_accum.append(buffer);

	// Parse request line
	if (_parseState == PS_REQUEST_LINE) {
		size_t pos = _accum.find("\r\n"); // Waiting for the end of the request line
		if (pos == std::string::npos) // If it's not complete we return waiting for more data
			return 0;
		std::string line = _accum.substr(0, pos);
		if (!parseRequestLine(line)) { // If we encounter a probleme parsing stop and we switch to building an error response
			_parseState = PS_ERROR;
			return -1;
		}
		_accum.erase(0, pos + 2); // remove the request line including the last \r\n\r\n from the accumulated data
		_parseState = PS_HEADERS;
	}

	// Once the request line is parsed, we can start parsing headers
	if (_parseState == PS_HEADERS) {
		size_t pos = _accum.find("\r\n\r\n");
		if (pos == std::string::npos)
			return 0;  // Waiting for the end of the headers
		std::string hdrs = _accum.substr(0, pos + 2); // include the last \r\n
		if (!parseHeaders(hdrs))
		{
			_parseState = PS_ERROR;
			return -1;
		}
		PutServer();
		_accum.erase(0, pos + 4); // remove the headers including the last \r\n\r\n
		_contentLenght = getContentLength(_headers);
		_parseState = (_contentLenght > 0 ? PS_BODY : PS_DONE); // If Content-Length is 0, we can directly go to DONE state
		if (_contentLenght > _server->_clientMaxBodySize) {
			_status = 413;
			_parseState = PS_ERROR;
			return -1;
		}
		if (_parseState == PS_DONE) {
			_isComplete = true;
			return 1;
		}
	}
	
	// If needed, parse the body
	if (_parseState == PS_BODY) {
		if (_method == "GET" || _method == "DELETE") {
			_isComplete = true;
			_parseState = PS_DONE;
			_status = 400; // Bad Request: GET and DELETE methods should not have a body
			return -1;
		}
		if (_accum.size() < static_cast<size_t>(_contentLenght))
			return 0;
		_body = _accum.substr(0, _contentLenght);
		_accum.erase(0, _contentLenght);
		_isComplete = true;
		_parseState = PS_DONE;
		return 1;
	}


	return 0;
}


/*-------------------------
	Request Line Parsing
---------------------------*/

/**
 * @brief Decodes a percent-encoded string.
 * 		This function decodes a string that may contain percent-encoded characters (e.g., "%20" for space).
 * 
 * @param encoded The percent-encoded string to decode.
 * @return std::string The decoded string.
 */
static	std::string percentDecode(const std::string& encoded)
{
	std::ostringstream decoded;
	size_t i = 0;

	while (i < encoded.length())
	{
		if (encoded[i] == '%')
		{
			if (i + 2 >= encoded.length())
				throw std::runtime_error("Invalid percent-encoding (truncated)");

			std::string hexStr = encoded.substr(i + 1, 2);
			int byteInt = 0;

			std::istringstream hexStream(hexStr);
			hexStream >> std::hex >> byteInt;

			if (hexStream.fail())
				throw std::runtime_error("Invalid hex in percent-encoding: " + hexStr);

			decoded << static_cast<char>(byteInt);
			i += 3;
		}
		else if (encoded[i] == '+')
		{
			decoded << ' '; // '+' means space in query strings
			i++;
		}
		else
			decoded << encoded[i++];
	}

	return decoded.str();
}

bool	HttpRequest::parseRequestLine(const std::string &line)
{
	std::istringstream	ss(line);
	std::string			method;
	std::string			request;
	std::string			protocol;

	ss >> method >> request >> protocol;

	if (method.empty() || request.empty() || protocol.empty())
	{
		_status = 400; // Bad Request: incomplete request line
		return false;
	}
	
	// Methods autorized by RFC
	const char* rfcMethods[] = {
		"GET", "POST", "PUT", "DELETE",
		"HEAD", "OPTIONS", "CONNECT", "TRACE", "PATCH"
	};
	bool isRfcMethod = false;
	for (int i = 0; i < 9; ++i)
	{
		if (method == rfcMethods[i])
		{
			isRfcMethod = true;
			break;
		}
	}

	if (!isRfcMethod)
	{
		if (method == "BREW")
			_status = 418; // I'm a teapot (RFC 2324) ;)
		else
			_status = 400; // Bad Request: unknown or invalid method
		return false;
	}

	// Supported methods
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		_status = 501; // Not Implemented
		return false;
	}

	_method = method;
	size_t pos = request.find('?');
	if (pos != std::string::npos)
	{
		_query = request.substr(pos + 1);
		request = request.substr(0, pos);
	}
	_uri = percentDecode(request);
	if (_uri.empty() || _uri[0] != '/' || _uri.find("..") != std::string::npos) // URI must start with '/' and not contain '..'
	{
		_status = 400; // Bad Request: invalid request URI
		return false;
	}

	if (protocol != "HTTP/1.0" && protocol != "HTTP/1.1")
	{
		_status = 505; // HTTP Version Not Supported
		return false;
	}

	_protocol = protocol;
	return true;
}


/*-------------------------
	Headers Parsing
---------------------------*/

bool	HttpRequest::parseHeaders(const std::string &headers)
{
	std::istringstream	ss(headers);
	std::string			line;

	while (std::getline(ss, line) && line != "\r" && !line.empty())
	{
		size_t	colon = line.find(':');

		if (colon == std::string::npos)
		{
			_status = 400; // Bad Request : missing colon in header
			return false;
		}

		std::string	key = line.substr(0, colon);
		std::string	value = line.substr(colon + 1);

		// trim initial spaces/tabs in value
		while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
			value.erase(0, 1);
		// trim trailing spaces/tabs in key and value
		while (!key.empty() && (key[key.size() - 1] == ' ' || key[key.size() - 1] == '\t'))
			key.erase(key.size() - 1);
		while (!value.empty() && (value[value.size() - 1] == ' ' || value[value.size() - 1] == '\t' || value[value.size() - 1] == '\r'))
			value.erase(value.size() - 1);

		if (key.empty())
		{
			_status = 400; // Bad Request : empty header name
			return false;
		}

		_headers[key] = value;

		if (key == "Connection")
			_connectionKeepAlive = (value == "keep-alive"); // Any other value means the connection is not kept alive
		else if (key == "Host")
			_host = value; // Store the Host header value
	}

	// If HTTP/1.1, Host header is mandatory RFC 7230
	// If the Host header is missing, we return a 400 Bad Request
	if (_protocol == "HTTP/1.1" && _host.empty())
	{
		_status = 400; // Bad Request : missing Host header
		return false;
	}

	return true;
}

void	HttpRequest::PutServer()
{
	// No Host header: use first available server
	if (_host.empty())
	{
		_server = _port->getServer(0);
		return;
	}

	size_t i = 0;
	t_server *server = NULL;
	while ((server = _port->getServer(i)))
	{
		if (std::find(server->_hosts.begin(), server->_hosts.end(), _host) != server->_hosts.end())
		{
			_server = server;
			return;
		}
		++i;
	}

	// No match found: default to first server
	_server = _port->getServer(0);
}


/*-------------------------
	Getters and Setters
---------------------------*/

size_t	HttpRequest::getContentLength(const std::map<std::string, std::string> &hdrs) {
	std::map<std::string,std::string>::const_iterator it = hdrs.find("Content-Length");
	if (it == hdrs.end()) return 0; // If Content-Length header is not present, return 0
	return std::strtoul(it->second.c_str(), NULL, 10);
}

bool HttpRequest::isComplete() const {
	return _parseState == PS_DONE;
}
