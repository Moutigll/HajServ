/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 16:52:05 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/15 16:56:20 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Response_HPP
# define Response_HPP

#include <iostream>
#include <sstream>

#include "Utils.hpp"

class Response
{
	public:
		Response();
		Response(const Response &src);
		Response &operator=(const Response &src);
		~Response();

		void	setStatusCode(int code);
		void	setHttpVersion(const std::string &version);
		void	setHeaders(const std::string &headers);
		void	setBody(const std::string &body);
		void	addHeader(const std::string &key, const std::string &value);

		std::string	serialize() const;	
	private:
		int			_statusCode;
		std::string	_httpVersion;
		std::string	_headers;
		std::string	_body;
};

#endif