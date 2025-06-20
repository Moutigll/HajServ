/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:11:03 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/20 22:42:13 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_TRANSACTION_HPP
#define HTTP_TRANSACTION_HPP

#include "Logger.hpp"
#include "HttpError.hpp"

class HttpTransaction {
	public:
		HttpTransaction();
		HttpTransaction(const HttpTransaction &other);
		HttpTransaction &operator=(const HttpTransaction &other);
		virtual ~HttpTransaction();

		virtual bool	isComplete() const = 0;
		void			log() const;

		int		getStatus() const;

	protected:
		std::string							_method;
		std::string							_request;
		std::string							_protocol;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		int									_status;
		bool								_isComplete;
};

#endif
