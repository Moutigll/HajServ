/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 21:20:52 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/20 15:47:30 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <fstream>
# include <iostream>
# include <map>
# include <sstream>
# include <string>
# include <vector>

# include "Location.hpp"
# include "Utils.hpp"
typedef struct s_server
{
	std::map<std::string, std::string>	_data;
	std::vector<Location>				_locations;
	std::map<int, std::string>			_errors;
	std::string							_root_error;
}	t_server;
class Config
{
	public:
		Config();
		Config(const Config &src);
		Config &operator=(const Config &src);
		~Config();

		bool	load(const std::string &path);
		bool	isLoaded() const;
		void	state() const;

		const std::string	&getGlobal(const std::string &key) const;
		const t_server		&getServerBlock(size_t index) const;

		size_t	getServerCount() const;
	private:
		bool	parseServerBlock(std::ifstream &file, int &line_number);
		bool	parseLocationBlock(std::ifstream &file, int &line_number, Location &location);
		bool	parseErrorBlock(std::ifstream &file, int &line_number, t_server &server);

		bool												_loaded;
		std::map<std::string, std::string>					_globals;
		std::vector<t_server>								_servers;
};

extern Config g_config;

#endif