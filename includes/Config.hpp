/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 04:01:24 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/30 14:55:10 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Utils.hpp"
#include "Location.hpp"

class Location;

typedef struct s_errors
{
	std::map<int, std::string>	_errors;
	std::string					_root_error;
}	t_errors;

typedef struct s_servers
{
	std::vector<std::string>			_methods;
	std::vector<std::string>			_names;
	std::map<std::string, std::string>	_data;
	t_errors							_errors;
	std::vector<Location>				_locations;
}	t_servers;

class Config
{
	public:
		Config();
		Config( Config const & );
		Config & operator=(Config const & );
		~Config();
		bool	parse(const std::string &filename);
		bool	GetGlobals(std::string &line, int &line_number);
	private:
		bool					_finished;
		bool					_log_connections;
		bool					_log_request;
		std::string				_log_level;
		std::vector<t_servers>	_servers;
};

#endif