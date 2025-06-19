/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTransaction.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:11:03 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/18 18:44:00 by ele-lean         ###   ########.fr       */
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
		void	log() const;

	protected:
		std::string							_method;
		std::string							_requestLine;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		HttpError							_status;
		bool								_isComplete;
};

#endif
