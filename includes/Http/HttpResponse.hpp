/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:26:55 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/18 19:11:29 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include "HttpTransaction.hpp"
#include "../Config.hpp"

class HttpResponse : public HttpTransaction {
	public:
		HttpResponse(const t_server &server);
		HttpResponse(const HttpResponse &other);
		HttpResponse &operator=(const HttpResponse &other);
		virtual ~HttpResponse();

		virtual bool isComplete() const;

		void	setStatus(const HttpError &status);
		void	setStatus(int code);
		void	addHeader(const std::string &name, const std::string &value);
		void	setBody(const std::string &body);

		void	construct();
		bool	sendResponse();

		HttpError	getStatus() const;
	private:
		t_server	_server;
		std::string	_response;
		int			readFd;
		int			socketFd;
};

#endif
