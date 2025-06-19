/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:16:25 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/18 18:33:51 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Http/HttpTransaction.hpp"

HttpTransaction::HttpTransaction()
	: _isComplete(false) {}

HttpTransaction::HttpTransaction(const HttpTransaction &other)
	: _requestLine(other._requestLine),
	  _headers(other._headers),
	  _body(other._body),
	  _isComplete(other._isComplete) {}

HttpTransaction &HttpTransaction::operator=(const HttpTransaction &other) {
	if (this != &other) {
		_requestLine = other._requestLine;
		_headers = other._headers;
		_body = other._body;
		_isComplete = other._isComplete;
	}
	return *this;
}

HttpTransaction::~HttpTransaction() {}

void HttpTransaction::log() const {
	std::string message = "HTTP Transaction:\n"
		"Request Line:\n\t" + _requestLine + "\n"
		"Headers:\n";
	for (const auto &header : _headers) {
		message += "\t" + header.first + ": " + header.second + "\n";
	}
	message += "Body:\n\t[" + _body + "]\n";

	g_logger.log(LOG_DEBUG, message);
}
