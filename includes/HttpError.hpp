/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpError.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:59:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/30 15:10:16 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef HTTPERROR_HPP
# define HTTPERROR_HPP

# include "Utils.hpp"

class HttpError
{
    public:
        HttpError();
        HttpError(int code);
        HttpError(const HttpError &src);
        HttpError &operator=(const HttpError &src);
        ~HttpError();

        std::string		getMessage(int code) const;
        int				getCode() const { return _code; }
    private:
        int	_code;

        static std::map<int, std::string>	_initHttpErrors();
        static std::map<int, std::string>	_HttpErrors;
};

#endif