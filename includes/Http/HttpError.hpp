/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpError.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:59:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/21 16:19:32 by ele-lean         ###   ########.fr       */
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

		void			setCode(int code);
		std::string		getMessage(int code) const;
		int				getCode() const { return _code; }
		std::string		getFilePath(void);
	private:
		int			_code;
		t_server	_server;

		static std::map<int, std::string>	_initHttpErrors();
		static std::map<int, std::string>	_HttpErrors;
};

#endif
