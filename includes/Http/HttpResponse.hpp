/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:26:55 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/25 21:12:23 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include "GetFiles.hpp"


const std::string VERSION = "HajServer/2.1.0";

class HttpResponse : public HttpTransaction {
	public:
		HttpResponse(const t_server &server);
		HttpResponse(const t_server &server, HttpRequest &request);
		HttpResponse(const HttpResponse &other);
		HttpResponse &operator=(const HttpResponse &other);
		virtual ~HttpResponse();

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
		 * @return t_buffer
		 *				A buffer containing the response data to be sent and its size.
		 *				The caller is responsible for freeing this memory.
		 *				Returns NULL in case of an error.
		 * 
		 * @warning Memory allocated with new[] must be freed by the caller using delete[].
		 */
		t_buffer	sendResponse();
		
		void	addHeader(const std::string &name, const std::string &value);
		void	setFilePath(const std::string &filePath);
		void	setStatus(const HttpError &status);
		void	setStatus(int code);
		void	setBody(const std::string &body);
		
		HttpError	getStatus() const;
		virtual bool	isComplete() const;
	private:
		t_server	_server; // Must be set, it is used to get the server configuration, paths and error pages
		std::string	_response; // Contains the request line and headers
		std::string	_filePath; // The file path to be served, if applicable
		int			_readFd; // File descriptor for reading file content, if applicable
		bool		_isHeadersSent; // Track if headers have been sent
		HttpError	_ErrorStatus;


		/**
		 * @brief Set HTTP headers for the file to serve.
		 * 
		 * If no file is set, tries to find one.  
		 * If there's an error status, sets the error page file.  
		 * Gets file info and sets "Content-Length" and "Content-Type" headers.  
		 * Handles errors by setting appropriate status and error pages.
		 */
		void		setFileHeaders();

		/**
		 * @brief Builds an error page when the requested resource is not found or an error occurs.
		 * 
		 */
		void		buildErrorPage();

		/**
		 * @brief Find and set the file path corresponding to the current URI.
		 * 
		 * - Finds the best matching location for the URI.
		 * 
		 * - Checks if the HTTP method is allowed.
		 * 
		 * - Builds the full file system path from the location root and URI.
		 * 
		 * - If the URI ends with '/', tries to find an index file or generates an autoindex page.
		 * 
		 * - Sets the appropriate HTTP status code (200, 403, 404, 405).
		 * 
		 * - Sets _filePath if a valid file is found.
		 */
		void		getFile();

		/**
		 * @brief Returns a buffer with the next chunk of the response body or file content.
		 * 
		 * @return t_buffer 
		 */
		t_buffer	getBody();
};

// HTML templates for error responses
#define HTML_HEADER "<!DOCTYPE html><html lang=\"en\"><head>" \
	"<meta charset=\"UTF-8\">" \
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
	"<title>Error</title>" \
	"<style>" \
	"body{margin:0;padding:0;font-family:'Segoe UI',sans-serif;" \
	"background:linear-gradient(to bottom,#a0d8ef,#ffffff);" \
	"display:flex;justify-content:center;align-items:center;height:100vh;text-align:center;color:#00334e;}" \
	".card{background:#fff;border-radius:20px;padding:2rem;box-shadow:0 8px 16px rgba(0,0,0,0.15);" \
	"max-width:500px;width:90%;}" \
	"h1{font-size:2rem;margin-bottom:0.5rem;}" \
	"p{font-size:1rem;line-height:1.5;}" \
	".footer{margin-top:1rem;font-size:0.8rem;color:#555;}" \
	"</style></head><body><div class=\"card\">"

#define HTML_FOOTER "<div class=\"footer\">" \
	"<p>HajServer/2.0.1</p></div></div></body></html>"

#endif
