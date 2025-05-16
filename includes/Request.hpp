/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 17:39:38 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/16 05:26:04 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <cstdlib>
#include <sstream>

#include "Colors.hpp"
#include "HttpCode.hpp"
#include "Utils.hpp"

class Request
{
	public:
		Request();
		Request(const std::string &request);
		Request(const Request &src);
		Request &operator=(const Request &src);
		~Request();

		HttpCode	parseRequest(const std::string &request);
		void		logRequest() const;

		const std::string	&getMethod() const;
		const std::string	&getUri() const;
		const std::string	&getHttpVersion() const;
		const std::string	&getHeader(const std::string &key) const;
		const std::string	&getBody() const;

		bool	hasHeader(const std::string &key) const;
		bool	isValid() const { return _is_valid; }

	private:
		HttpCode	parseRequestLine(std::istringstream &stream);
		HttpCode	parseHeaders(std::istringstream &stream);
		HttpCode	parseBody(std::istringstream &stream);

		std::string							_method;
		std::string							_uri;
		std::string							_httpVersion;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		bool								_is_valid;
};

#endif
