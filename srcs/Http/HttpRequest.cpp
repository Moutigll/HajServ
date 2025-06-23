/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 21:05:28 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/23 23:54:44 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Http/HttpRequest.hpp"

HttpRequest::HttpRequest()
	: HttpTransaction(),
	  _parse_state(PS_REQUEST_LINE),
	  _content_length(0)
{
	_isComplete = false;
}

HttpRequest::HttpRequest(const HttpRequest &other)
	: HttpTransaction(other),
	  _parse_state(other._parse_state),
	  _accum(other._accum),
	  _content_length(other._content_length)
{
}

HttpRequest &HttpRequest::operator=(const HttpRequest &other) {
	if (this != &other) {
		HttpTransaction::operator=(other);
		_parse_state   = other._parse_state;
		_accum         = other._accum;
		_content_length= other._content_length;
	}
	return *this;
}

HttpRequest::~HttpRequest() {}

bool HttpRequest::isComplete() const
{
	return _parse_state == PS_DONE;
}

int HttpRequest::parse(const char *buffer)
{
	if (_parse_state == PS_ERROR || _parse_state == PS_DONE)
		return _parse_state == PS_DONE ? 1 : -1;

	_accum.append(buffer);
	std::cout << "Accumulated data: " << _accum << std::endl; // Debugging output

	if (_parse_state == PS_REQUEST_LINE) {
		size_t pos = _accum.find("\r\n");
		if (pos == std::string::npos)
			return 0;  // Waiting for the end of the request line
		std::string line = _accum.substr(0, pos);
		if (!parseRequestLine(line)) {
			_parse_state = PS_ERROR;
			return -1;
		}
		_accum.clear(); // remove the request line including the last \r\n
		_parse_state = PS_HEADERS;
	}

	if (_parse_state == PS_HEADERS) {
		size_t pos = _accum.find("\r\n\r\n");
		if (pos == std::string::npos)
			return 0;  // Waiting for the end of the headers
		std::string hdrs = _accum.substr(0, pos + 2); // include the last \r\n
		if (!parseHeaders(hdrs)) {
			_parse_state = PS_ERROR;
			return -1;
		}
		_accum.erase(0, pos + 4); // remove the headers including the last \r\n\r\n
		_content_length = getContentLength(_headers);
		_parse_state = (_content_length > 0 ? PS_BODY : PS_DONE);
		if (_parse_state == PS_DONE) {
			_isComplete = true;
			return 1;
		}
	}
	
	if (_parse_state == PS_BODY) {
		if (_accum.size() < _content_length)
			return 0;
		std::string body = _accum.substr(0, _content_length);
		if (!parseBody(body)) {
			_parse_state = PS_ERROR;
			return -1;
		}
		_accum.erase(0, _content_length);
		_parse_state = PS_DONE;
		_isComplete = true;
		return 1;
	}

	return 0;
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
		_status = 400; // Bad Request : imcomplet request line
		return false;
	}

	if (protocol != "HTTP/1.0" && protocol != "HTTP/1.1")
	{
		_status = 505; // HTTP Version Not Supported
		return false;
	}

	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		_status = 405; // Method Not Allowed
		return false;
	}
	_method = method;
	_request = percentDecode(request); // Decode percent-encoded characters in the request URI
	if (_request.empty() || _request[0] != '/' || _request.find("..") != std::string::npos)
	{
		_status = 400; // Bad Request : empty request URI
		return false;
	}
	std::cout << "Request URI: " << _request << std::endl; // Debugging output
	_protocol = protocol;
	return true;
}


bool	HttpRequest::parseHeaders(const std::string &headers)
{
	std::istringstream	ss(headers);
	std::string			line;
	bool				has_host = false;

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

		toLowercase(value);

		// trim initial spaces/tabs in value
		while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
			value.erase(0, 1);
		while (!key.empty() && (key[key.size() - 1] == ' ' || key[key.size() - 1] == '\t'))
			key.erase(key.size() - 1);

		if (key.empty())
		{
			_status = 400; // Bad Request : empty header name
			return false;
		}

		_headers[key] = value;

		if (key == "Connection" && value == "keep-alive")
			_connectionKeepAlive = true; // Any other value means the connection is not kept alive

		if (_protocol == "HTTP/1.1" && key == "Host")
			has_host = true;
		if (key == "Host")
			_host = value; // Store the Host header value
	}

	if (_protocol == "HTTP/1.1" && !has_host)
	{
		_status = 400; // Bad Request : missing Host header
		return false;
	}

	return true;
}


bool HttpRequest::parseBody(const std::string &body)
{
	_body = body;
	return true;
}

size_t HttpRequest::getContentLength(const std::map<std::string, std::string> &hdrs) {
	std::map<std::string,std::string>::const_iterator it = hdrs.find("Content-Length");
	if (it == hdrs.end()) return 0;
	return std::strtoul(it->second.c_str(), NULL, 10);
}

void HttpRequest::setPort(Port *port) {
	_port = port;
}