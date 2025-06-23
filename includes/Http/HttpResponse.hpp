/* ************************************************************************** */
/*																			*/
/*														:::	  ::::::::   */
/*   HttpResponse.hpp								   :+:	  :+:	:+:   */
/*													+:+ +:+		 +:+	 */
/*   By: ele-lean <ele-lean@student.42.fr>		  +#+  +:+	   +#+		*/
/*												+#+#+#+#+#+   +#+		   */
/*   Created: 2025/06/18 18:26:55 by ele-lean		  #+#	#+#			 */
/*   Updated: 2025/06/23 21:33:58 by ele-lean		 ###   ########.fr	   */
/*																			*/
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include "HttpTransaction.hpp"
#include "HttpRequest.hpp"
#include "../Config.hpp"

class HttpResponse : public HttpTransaction {
	public:
		HttpResponse(const t_server &server);
		HttpResponse(const t_server &server, HttpRequest &request);
		HttpResponse(const HttpResponse &other);
		HttpResponse &operator=(const HttpResponse &other);
		virtual ~HttpResponse();

		virtual bool isComplete() const;

		void	setStatus(const HttpError &status);
		void	setStatus(int code);
		void	addHeader(const std::string &name, const std::string &value);
		void	setBody(const std::string &body);
		void	setFilePath(const std::string &filePath);

		/**
		 * @brief Constructs the HTTP response headers and prepares the body.
		 * This function initializes the response string with the HTTP status line,
		 * sets the Content-Length header if a body is present,
		 * and adds the Date and Server headers.
		 * If the response status is an error (4xx or 5xx), it sets the appropriate
		 * error headers and prepares the error response.
		 */
		void	construct();

		/**
		 * @brief Sends the HTTP response including headers and body data.
		 * 
		 * This function first sends the HTTP headers along with the beginning of the body
		 * (or the beginning of the file if reading from a file descriptor), limited by
		 * the server's maximum body size (_server._max_body_size). This optimization reduces
		 * the number of epoll calls by sending headers and part of the body in a single chunk.
		 * 
		 * On the first call:
		 * - Constructs the response headers if not already done.
		 * - Sends headers concatenated with the first chunk of the body or file content.
		 * 
		 * On subsequent calls:
		 * - Sends the remaining body data or reads the next chunk from the file descriptor.
		 * 
		 * @return char* Pointer to a dynamically allocated buffer containing the data to send.
		 *				The caller is responsible for freeing this memory.
		 *				Returns NULL in case of an error.
		 * 
		 * @warning Memory allocated with new[] must be freed by the caller using delete[].
		 */
		char	*sendResponse();

		HttpError	getStatus() const;
	private:
		t_server	_server;
		std::string	_response;
		HttpError	_ErrorStatus;
		std::string	_filePath;
		bool		_isHeadersSent;
		int			_readFd;

		/**
		 * @brief Set the headers for a file response.
		 * If the file exists, it sets the Content-Length and Content-Type headers.
		 * If the file does not exist, it sets a 404 Not Found status and prepares an error response.
		 */
		void		setFileHeaders();

		char		*getBody();
};

#endif
