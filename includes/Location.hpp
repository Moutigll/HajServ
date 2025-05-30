/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 23:49:57 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/30 14:17:47 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Utils.hpp"

class Location
{
	private:
		std::string							_path;
		std::map<std::string, std::string>	_loc_data;
		int									_return_code;
		std::string							_return_uri;
		bool								_autoindex;
		std::vector<std::string>			_methods;
		std::vector<std::string>			_try_files;
	public:
		Location();
		Location( const Location & );
		Location &operator=( const Location & );
		~Location();
};

#endif