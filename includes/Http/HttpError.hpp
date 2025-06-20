/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpError.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:59:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/20 19:32:58 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef HTTPERROR_HPP
# define HTTPERROR_HPP

# include "Logger.hpp"

class HttpError
{
	public:
		HttpError();
		HttpError(int code);
		HttpError(const t_server &server);
		HttpError(const HttpError &src);
		HttpError(int code, const t_server &server);
		HttpError &operator=(const HttpError &src);
		~HttpError();

		std::string		getMessage(int code) const;
		int				getCode() const { return _code; }
		int				getFd( void );
	private:
		int			_code;
		t_server	_server;

		static std::map<int, std::string>	_initHttpErrors();
		static std::map<int, std::string>	_HttpErrors;
};

#endif