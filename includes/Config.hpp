/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 04:01:24 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/30 20:02:35 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Utils.hpp"

typedef struct s_location
{
	std::string							_path;
	std::map<std::string, std::string>	_loc_data;
	int									_return_code;
	std::string							_return_uri;
	bool								_autoindex;
	std::vector<std::string>			_methods;
	std::vector<std::string>			_try_files;
}	t_location;

typedef struct s_servers
{
	std::vector<std::string>			_methods;
	std::vector<std::string>			_names;
	std::vector<std::string>			_indexes;
	std::map<std::string, std::string>	_data;
	std::map<int, std::string>			_errors;
	std::string							_root_error;
	std::vector<t_location>				_locations;
}	t_servers;

class Config
{
	public:
		Config();
		Config( Config const & );
		Config & operator=(Config const & );
		~Config();
		bool	parse(const std::string &filename);
		const t_servers		&getServerBlock(size_t index) const;

		size_t	getServerCount() const;
	private:
		bool	parseGlobals(std::string &line, int &line_number);
		bool	parseServer(std::ifstream &file, int &line_number);
		bool	parseLocation(std::ifstream &file, int &line_number, t_location &server);
		bool	parseErrors(std::ifstream &file, int &line_number, t_servers &server);
		bool	parseVectors(std::string &line, t_servers &server);

		bool					_finished;
		bool					_log_connections;
		bool					_log_request;
		std::string				_log_level;
		std::vector<t_servers>	_servers;
};

#endif