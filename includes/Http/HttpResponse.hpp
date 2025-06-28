/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:26:55 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/29 00:01:08 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include "../CgiHandler.hpp"

const std::string VERSION = "HajServ/2.2.5";

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
		
		HttpError		getStatus() const;
		virtual bool	isComplete() const;
	private:
		t_server	_server; // Must be set, it is used to get the server configuration, paths and error pages
		std::string	_response; // Contains the request line and headers
		std::string	_filePath; // The file path to be served, if applicable
		int			_readFd; // File descriptor for reading file content, if applicable
		bool		_isHeadersSent; // Track if headers have been sent
		HttpError	_ErrorStatus;
		CgiHandler	*_cgiHandler; // Handler for CGI execution
		bool		_isCgiComplete; // Track if CGI execution is complete


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
		 * @brief Checks if the given file path is a CGI script based on the location configuration.
		 * @param loc The location configuration to check against.
		 * @param filepath The file path to check.
		 * @return Returns the CGI binary path if the file is a CGI script, or an empty string if not.
		 */
		std::string	isCgiFile(const t_location *loc, const std::string &filepath);

		/**
		 * @brief Handles the CGI execution and reads the output.
		 * This function checks if the CGI handler is set, reads from the CGI output pipe,	
		 * and updates the response body with the CGI output.
		 * If the CGI execution is complete, it sets the status code based on the CGI handler's output.
		 * If the CGI execution times out or fails, it sets the appropriate error status.
		 * @note This function should be called repeatedly until the CGI execution is complete.
		 */
		void	handleCgi();


		/**
		 * @brief Extract CGI headers from the body and store them in _headers.
		 * 
		 * This function parses the headers at the beginning of a CGI body,
		 * stores them line by line in _headers and removes them from the body.
		 * Handle special headers such as status code, content type, and content length.
		 * 
		 * @param body The CGI response body to parse.
		 */
		void	extractCgiHeaders(const std::string &body);

		/**
		 * @brief Returns a buffer with the next chunk of the response body or file content.
		 * 
		 * @return t_buffer 
		 */
		t_buffer	getBody();

		/**
		 * @brief Generates environment variables for CGI execution.
		 * @param filepath The file path to the CGI script.
		 * @param envMap The map to populate with environment variables.
		 * This function sets common CGI environment variables such as GATEWAY_INTERFACE, SERVER_PROTOCOL,
		 * REQUEST_METHOD, SCRIPT_FILENAME, SCRIPT_NAME, SERVER_SOFTWARE, QUERY_STRING, etc.
		 * It also includes any additional environment variables that may be required for the CGI script.
		*/
		void generateEnvMap(const std::string &filepath, std::map<std::string, std::string> &envMap);
};

#endif
