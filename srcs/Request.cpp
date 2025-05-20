/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 17:43:10 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/20 12:02:18 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Request.hpp"

Request::Request() {}

Request::Request(const std::string &request)
{
	std::string	req(request);

	parseRequest(req);
}

Request::Request(const Request &src)
{
	*this = src;
}

Request	&Request::operator=(const Request &src)
{
	if (this != &src)
	{
		_method = src._method;
		_uri = src._uri;
		_httpVersion = src._httpVersion;
		_headers = src._headers;
		_body = src._body;
	}
	return (*this);
}

Request::~Request() {}

HttpCode	Request::parseRequest(std::string const &raw_request)
{
	std::istringstream	stream(raw_request);
	HttpCode			ret;

	_is_valid = false;

	ret = parseRequestLine(stream);
	if (ret.getCode() != 200)
		return (ret);

	ret = parseHeaders(stream);
	if (ret.getCode() != 200)
		return (ret);

	ret = parseBody(stream);
	if (ret.getCode() != 200)
		return (ret);

	_is_valid = true;
	return (HttpCode(200));
}

static bool	is_valid_uri_char(char c)
{
	if (std::isalnum(static_cast<unsigned char>(c)))
		return (true);
	if (c == '-' || c == '_' || c == '.' || c == '/' || c == '~'
		|| c == '%' || c == '?' || c == '&' || c == '=')
		return (true);
	return (false);
}

static bool	validate_uri(const std::string &uri)
{
	size_t	i;

	if (uri.empty() || uri[0] != '/')
		return (false);
	i = 0;
	while (i < uri.size())
	{
		if (!is_valid_uri_char(uri[i]))
			return (false);
		i++;
	}
	return (true);
}

static bool isInArray(const char *arr[], size_t size, const std::string &val)
{
	size_t i = 0;
	while (i < size)
	{
		if (val == arr[i])
			return true;
		i++;
	}
	return false;
}

HttpCode Request::parseRequestLine(std::istringstream &stream)
{
	std::string line;
	std::istringstream ls;

	if (!std::getline(stream, line) || line.empty())
		return (HttpCode(400));
	if (line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	ls.str(line);
	ls >> _method >> _uri >> _httpVersion;

	if (ls.fail()
		|| _method.empty()
		|| _uri.empty()
		|| _httpVersion.empty())
		return (HttpCode(400));

	if (_httpVersion != "HTTP/1.0"
		&& _httpVersion != "HTTP/1.1")
		return (HttpCode(505)); // HTTP Version Not Supported

	const char *validMethods[] = {
		"GET", "HEAD", "POST",
		"PUT", "DELETE", "CONNECT",
		"OPTIONS", "TRACE", "PATCH"};
	const size_t validMethodsSize = sizeof(validMethods) / sizeof(validMethods[0]);
	const char *allowedMethods[] = {
		"GET"
	};
	const size_t allowedMethodsSize = sizeof(allowedMethods) / sizeof(allowedMethods[0]);
	const char *notImplementedMethods[] = {
		"POST", "PUT", "DELETE"};
	const size_t notImplementedMethodsSize = sizeof(notImplementedMethods) / sizeof(notImplementedMethods[0]);

	if (!isInArray(validMethods, validMethodsSize, _method))
		return (HttpCode(400)); // method unknown -> 400 Bad Request

	if (!isInArray(allowedMethods, allowedMethodsSize, _method))
	{
		if (isInArray(notImplementedMethods, notImplementedMethodsSize, _method))
			return (HttpCode(501)); // method known but not implemented
		else
			return (HttpCode(405)); // method known but not allowed
	}

	if (!validate_uri(_uri))
		return (HttpCode(400));

	return (HttpCode(200));
}

HttpCode Request::parseHeaders(std::istringstream &stream)
{
	std::string	line;
	size_t		pos;
	std::string	key;
	std::string	val;
	size_t		i;

	while (std::getline(stream, line))
	{
		if (line == "\r" || line.empty())
		{
			if (_httpVersion == "HTTP/1.1" && _headers.find("host") == _headers.end())
				return (HttpCode(400));

			return (HttpCode(200));
		}
		if (line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		pos = line.find(':');
		if (pos == std::string::npos)
			return (HttpCode(400));

		key = toLower(line.substr(0, pos));
		val = line.substr(pos + 1);

		i = 0;
		while (i < val.size() && isspace(static_cast<unsigned char>(val[i])))
			i++;
		val.erase(0, i);

		if (key.empty() || val.empty())
			continue;

		_headers[key] = val;
	}
	return (HttpCode(400));
}

HttpCode	Request::parseBody(std::istringstream &stream)
{
	std::string	content_length_str;
	size_t		content_length;
	size_t		total_read;
	char		c;
	char*		endptr;

	content_length_str = "";
	if (_headers.find("content-length") != _headers.end())
		content_length_str = _headers["content-length"];
	if (!content_length_str.empty())
	{
		size_t	i = 0;
		while (i < content_length_str.size())
		{
			if (!isdigit(static_cast<unsigned char>(content_length_str[i])))
				return (HttpCode(400));
			i++;
		}
		content_length = static_cast<size_t>(std::strtoul(content_length_str.c_str(), &endptr, 10));
		if (*endptr != '\0')
			return (HttpCode(400));
	}
	else
		content_length = 0;

	_body.clear();
	total_read = 0;
	while (total_read < content_length && stream.get(c))
	{
		if (c == '\r')
		{
			if (stream.peek() == '\n')
				stream.get(c);
			_body += '\n';
		}
		else
			_body += c;
		total_read++;
	}
	if (total_read != content_length)
		return (HttpCode(400));

	return (HttpCode(200));
}

void	Request::logRequest() const
{
	std::cout << CYAN << "==========================================" << RESET << std::endl;
	std::cout << CYAN << "                HTTP REQUEST              " << RESET << std::endl;
	std::cout << CYAN << "==========================================" << RESET << std::endl;

	std::cout << GREEN << "Method:        " << RESET << _method << std::endl;
	std::cout << GREEN << "URI:           " << RESET << _uri << std::endl;
	std::cout << GREEN << "HTTP Version:  " << RESET << _httpVersion << std::endl;

	std::cout << YELLOW << "--------------- HEADERS ------------------" << RESET << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		it != _headers.end(); ++it)
		std::cout << YELLOW << it->first << ": " << RESET << it->second << std::endl;

	std::cout << YELLOW << "---------------- BODY ---------------------" << RESET << std::endl;
	std::cout << GREEN << "Body size:     " << RESET << _body.length() << " bytes" << std::endl;

	if (!_body.empty())
		std::cout << RED << _body << RESET << std::endl;
	else
		std::cout << RED << "(empty)" << RESET << std::endl;

	std::cout << CYAN << "==========================================" << RESET << std::endl;
}

const std::string	&Request::getMethod() const {return (_method);}
const std::string	&Request::getUri() const {return (_uri);}
const std::string	&Request::getHttpVersion() const {return (_httpVersion);}
const std::string	&Request::getBody() const {return (_body);}

bool Request::hasHeader(const std::string &key) const {return (_headers.find(toLower(key)) != _headers.end());}

const std::string	&Request::getHeader(const std::string &key) const
{
	static std::string	empty = "";

	std::map<std::string, std::string>::const_iterator	it = _headers.find(toLower(key));
	if (it != _headers.end())
		return (it->second);
	return (empty);
}
