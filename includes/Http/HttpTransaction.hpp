/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:11:03 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/24 16:01:33 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_TRANSACTION_HPP
#define HTTP_TRANSACTION_HPP

#include "../Utils.hpp"

class HttpTransaction {
	public:
		HttpTransaction();
		HttpTransaction(const HttpTransaction &other);
		HttpTransaction &operator=(const HttpTransaction &other);
		virtual ~HttpTransaction();

		virtual bool	isComplete() const = 0;
		void			log() const;

		int			getStatus() const;
		std::string	getMethod() const;
		std::string	getRequest() const;
		std::string	getProtocol() const;
		bool		isConnectionKeepAlive() const;

	protected:
		std::string							_method;
		std::string							_request;
		bool								_connectionKeepAlive;
		std::string							_protocol;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		int									_status;
		bool								_isComplete;
		t_server							*_server;
};

#endif
