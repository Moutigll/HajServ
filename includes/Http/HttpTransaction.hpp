/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:11:03 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/27 16:58:28 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_TRANSACTION_HPP
#define HTTP_TRANSACTION_HPP

#include "../Utils.hpp"

/**
 * @class HttpTransaction
 * @brief Abstract base class representing an HTTP transaction (request or response).
 *
 * This class provides common members and interface for HTTP requests and responses.
 * It manages HTTP method, URI, protocol, headers, body, status code, and connection state.
 * 
 * Derived classes must implement the pure virtual method isComplete() to indicate 
 * whether the transaction is fully received or ready.
 */
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
		std::string getTBody() const;
		std::map<std::string, std::string> getHeaders() const;
		bool		isConnectionKeepAlive() const;
		std::string getQuery() const;

	protected:
		std::string							_method;
		std::string							_uri; // The request URI
		std::string							_query; // Host header value
		bool								_connectionKeepAlive;
		std::string							_protocol;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		int									_status;
		bool								_isComplete;
		t_server							*_server;
};

#endif
