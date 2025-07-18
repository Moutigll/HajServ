/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:24:31 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/28 11:08:55 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "HttpTransaction.hpp"
#include "../Port.hpp"

class HttpRequest : public HttpTransaction {
	public:
		HttpRequest(Port *port);
		HttpRequest(const HttpRequest &other);
		HttpRequest &operator=(const HttpRequest &other);
		virtual ~HttpRequest();

		/**
		 * @brief Parse the HTTP request from the given buffer.
		 * If an error occurs, the status contains the corresponding error code.
		 * It can process the request line, headers, and body in sequence.
		 * @param buffer The buffer containing the HTTP request data.
		 * @return Returns 1 if the request is complete, 0 if more data is needed, or -1 if an error occurred.
		 */	
		int				parse(const char *buffer);

		/**
		 * @brief Verify if the request is complete.
		 * @return true if the request is complete, false otherwise.
		 */
		virtual bool	isComplete() const;
	private:
		enum ParseState {
			PS_REQUEST_LINE,
			PS_HEADERS,
			PS_BODY,
			PS_DONE,
			PS_ERROR
		};

		ParseState			_parseState; // Store the current parsing state allowing to recieve data in chunks
		std::string			_accum; // Data not yet parsed
		std::string			_host; // Host header value
		size_t				_contentLenght;
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
		 * @brief Return the content length from headers.
		 * If the Content-Length header is not present, returns 0.
		 * @param hdrs Headers map.
		 * @return Content length as size_t.
		 */
		static size_t	getContentLength(const std::map<std::string, std::string> &hdrs);

		/**
		 * @brief Selects the appropriate server configuration based on the Host header.
		 *
		 * This function iterates over all server configurations associated with a given port
		 * and selects the one that matches the Host header in the HTTP request. If no Host
		 * header is present or no matching host is found, the first server (index 0) is used
		 * by default.
		 *
		 * @note This function assumes that _port is already initialized and points to a valid
		 *       Port object containing server configurations.
		 *
		 * @warning If _port is null or contains no servers, behavior is undefined.
		 */
		void	PutServer();
};

#endif
