/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpError.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:59:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/25 16:41:20 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef HTTPERROR_HPP
# define HTTPERROR_HPP

# include "../Utils.hpp"

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
		void			setServer(const t_server &server);

		/**
		 * @brief Returns the HTTP error message corresponding to the given code.
		 * 
		 * @param code The HTTP error code (e.g., 404, 500).
		 * @return std::string 
		 */
		std::string		getMessage(int code) const;
		int				getCode() const;

		/**
		 * @brief Get the File Path if the error has a t_server associated.
		 * 		It searches for the error code in the server's error map.
		 * 
		 * @return std::string the file path associated with the error code,
		 */
		std::string		getFilePath(void);
	private:
		int			_code;
		t_server	_server;

		static std::map<int, std::string>	_initHttpErrors();
		static std::map<int, std::string>	_HttpErrors;
};

#endif
