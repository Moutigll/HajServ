/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:24:31 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/24 16:03:12 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "HttpTransaction.hpp"
#include "Port.hpp"
#include <string>
#include <map>

class HttpRequest : public HttpTransaction {
public:
	HttpRequest();
	HttpRequest(const HttpRequest &other);
	HttpRequest &operator=(const HttpRequest &other);
	virtual ~HttpRequest();

	/**
	 * @brief Verify if the request is complete.
	 * @return true if the request is complete, false otherwise.
	 */
	virtual bool	isComplete() const;

	/**
	 * @brief Parse the HTTP request from the given buffer.
	 * If an error occurs, the status contains the corresponding error code.
	 * It can process the request line, headers, and body in sequence.
	 * @param buffer The buffer containing the HTTP request data.
	 * @return Returns 1 if the request is complete, 0 if more data is needed, or -1 if an error occurred.
	 */	
	int				parse(const char *buffer);

	/**
	 * @brief Set the port on which the request was received.
	 * @param port Pointer to the Port object.
	 */
	void			setPort(Port *port);

private:
	enum ParseState {
		PS_REQUEST_LINE,
		PS_HEADERS,
		PS_BODY,
		PS_DONE,
		PS_ERROR
	};

	ParseState			_parse_state;
	std::string			_accum; // Data not yet parsed
	std::string			_host; // Host header value
	size_t				_content_length;
	Port				*_port; // Port on which the request was received
	
	/**
	 * @brief Parse the request line from the given line.
	 * The line should contain the method, request URI, and protocol version.
	 * @param line The request line to parse.
	 * @return true if the request line is valid, false otherwise.
	 */
	bool	parseRequestLine(const std::string &line);

	/**
	 * @brief Parse the headers from the given headers string.
	 * The headers should be in the format "Header-Name: value".
	 * @param headers The headers string to parse.
	 * @return true if the headers are valid, false otherwise.
	 */
	bool	parseHeaders(const std::string &headers);

	/**
	 * @brief Parse the body from the given body string.
	 * The body is expected to be in the format defined by the Content-Length header.
	 * @param body The body string to parse.
	 * @return true if the body is valid, false otherwise.
	 */
	bool	parseBody(const std::string &body);

	/**
	 * @brief Return the content length from headers.
	 * If the Content-Length header is not present, returns 0.
	 * @param hdrs Headers map.
	 * @return Content length as size_t.
	 */
	static size_t	getContentLength(const std::map<std::string, std::string> &hdrs);
	void	PutServer();
};

#endif
