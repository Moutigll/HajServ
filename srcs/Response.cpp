/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 16:56:38 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/15 16:57:36 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Response.hpp"

Response::Response() : _statusCode(200), _httpVersion("HTTP/1.1"), _headers(""), _body("") {}

Response::Response(const Response &src)
{
	*this = src;
}

Response &Response::operator=(const Response &src)
{
	if (this != &src)
	{
		this->_statusCode = src._statusCode;
		this->_httpVersion = src._httpVersion;
		this->_headers = src._headers;
		this->_body = src._body;
	}
	return *this;
}

Response::~Response() {}

void	Response::setStatusCode(int code) {this->_statusCode = code;}
void	Response::setHttpVersion(const std::string &version) {this->_httpVersion = version;}
void	Response::setHeaders(const std::string &headers) {this->_headers = headers;}
void	Response::setBody(const std::string &body) {this->_body = body;}

void	Response::addHeader(const std::string &key, const std::string &value)
{
	if (this->_headers.empty())
		this->_headers += key + ": " + value;
	else
		this->_headers += "\r\n" + key + ": " + value;
}

std::string	Response::serialize(void) const
{
	std::string			response;
	std::ostringstream	statusCodeStream;
	std::ostringstream	contentLengthStream;

	statusCodeStream << this->_statusCode;
	contentLengthStream << this->_body.length();
	response += this->_httpVersion + " " + statusCodeStream.str() + "\r\n";
	response += "Server: HajServ/1.0\r\n";
	response += "Date: " + getHttpDate() + "\r\n";
	response += "Content-Length: " + contentLengthStream.str() + "\r\n";
	response += this->_headers + "\r\n\r\n";
	response += this->_body;

	return (response);
}