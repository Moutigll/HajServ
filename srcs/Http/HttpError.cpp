/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpError.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/21 15:13:09 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/25 16:40:13 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/HttpError.hpp"

std::map<int, std::string> HttpError::_HttpErrors = HttpError::_initHttpErrors();

HttpError::HttpError() : _code(200) {}

HttpError::HttpError(int code) : _code(code) {}

HttpError::HttpError(const t_server &server) : _code(200), _server(server) {}

HttpError::HttpError(const HttpError &src) : _code(src._code) {}

HttpError::HttpError(int code, const t_server &server) : _code(code), _server(server) {}

HttpError &HttpError::operator=(const HttpError &src)
{
	if (this != &src)
		_code = src._code;
	return *this;
}

HttpError::~HttpError() {}

void HttpError::setCode(int code)
{
	_code = code;
}

std::map<int, std::string> HttpError::_initHttpErrors()
{
	std::map<int, std::string> codes;
	codes[100] = "Continue";
	codes[101] = "Switching Protocols";
	codes[200] = "OK";
	codes[201] = "Created";
	codes[202] = "Accepted";
	codes[203] = "Non-Authoritative Information";
	codes[204] = "No Content";
	codes[205] = "Reset Content";
	codes[206] = "Partial Content";
	codes[300] = "Multiple Choices";
	codes[301] = "Moved Permanently";
	codes[302] = "Found";
	codes[303] = "See Other";
	codes[304] = "Not Modified";
	codes[305] = "Use Proxy";
	codes[307] = "Temporary Redirect";
	codes[400] = "Bad Request";
	codes[401] = "Unauthorized";
	codes[402] = "Payment Required";
	codes[403] = "Forbidden";
	codes[404] = "Not Found";
	codes[405] = "Method Not Allowed";
	codes[406] = "Not Acceptable";
	codes[407] = "Proxy Authentication Required";
	codes[408] = "Request Timeout";
	codes[409] = "Conflict";
	codes[410] = "Gone";
	codes[411] = "Length Required";
	codes[412] = "Precondition Failed";
	codes[413] = "Payload Too Large";
	codes[414] = "URI Too Long";
	codes[415] = "Unsupported Media Type";
	codes[416] = "Range Not Satisfiable";
	codes[417] = "Expectation Failed";
	codes[418] = "I'm a teapot";
	codes[426] = "Upgrade Required";
	codes[500] = "Internal Server Error";
	codes[501] = "Not Implemented";
	codes[502] = "Bad Gateway";
	codes[503] = "Service Unavailable";
	codes[504] = "Gateway Timeout";
	codes[505] = "HTTP Version Not Supported";
	return codes;
}

std::string	HttpError::getMessage(int code) const
{
	std::map<int, std::string>::const_iterator it = _HttpErrors.find(code);
	if (it != _HttpErrors.end())
		return it->second;
	return std::string("Unknown Status");
}

std::string	HttpError::getFilePath( void )
{
	std::string error_page = _server._root_error;
	std::map<int, std::string>::const_iterator it = _server._errors.find(this->_code);
	if (it != _server._errors.end())
	{
		if (error_page[error_page.length() - 1] != '/')
			error_page += '/';
		return error_page + it->second;
	}
	return "";
}

void HttpError::setServer(const t_server &server)
{
	_server = server;
}

int HttpError::getCode() const
{
	return _code;
}
