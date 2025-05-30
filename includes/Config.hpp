/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 04:01:24 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/30 17:53:40 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Utils.hpp"
#include "Location.hpp"
#include <vector>

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
	std::vector<std::string>			_indexes;
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
		bool	parseGlobals(std::string &line, int &line_number);
		bool	parseServer(std::ifstream &file, int &line_number);
		bool	parseLocation(std::string &line, int &line_number);
		bool	parseErrors(std::string &line, int &line_number);
		bool	parseVectors(std::string &line, int &line_number, t_servers &server);
	private:
		bool					_finished;
		bool					_log_connections;
		bool					_log_request;
		std::string				_log_level;
		std::vector<t_servers>	_servers;
};

#endif