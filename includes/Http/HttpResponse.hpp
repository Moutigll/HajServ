/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:26:55 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/21 16:52:49 by ele-lean         ###   ########.fr       */
/*                                                                            */
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

		void	construct();
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
};

#endif
