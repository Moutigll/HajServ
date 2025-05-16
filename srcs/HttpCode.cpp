/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpCode.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 03:36:42 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/16 05:20:45 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpCode.hpp"


std::map<int, std::string> HttpCode::_httpCodes = HttpCode::_initHttpCodes();

HttpCode::HttpCode() : _code(200) {}

HttpCode::HttpCode(int code) : _code(code) {}

HttpCode::HttpCode(const HttpCode &src) : _code(src._code) {}

HttpCode &HttpCode::operator=(const HttpCode &src)
{
    if (this != &src)
        _code = src._code;
    return *this;
}

HttpCode::~HttpCode() {}

std::map<int, std::string> HttpCode::_initHttpCodes()
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
    codes[426] = "Upgrade Required";
    codes[500] = "Internal Server Error";
    codes[501] = "Not Implemented";
    codes[502] = "Bad Gateway";
    codes[503] = "Service Unavailable";
    codes[504] = "Gateway Timeout";
    codes[505] = "HTTP Version Not Supported";
    return codes;
}

std::string HttpCode::getMessage(int code) const
{
    std::map<int, std::string>::const_iterator it = _httpCodes.find(code);
    if (it != _httpCodes.end())
        return it->second;
    return std::string("Unknown Status");
}
