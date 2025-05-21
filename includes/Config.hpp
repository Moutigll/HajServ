/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:28:23 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/21 15:35:07 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP
# include <fstream>
# include <iostream>
# include <map>
# include <sstream>
# include <string>
# include <vector>

typedef struct s_server
{
	std::map<std::string, std::string>	_data;
	std::map<int, std::string>			_error_pages;
	std::string							_root_error;
}	t_server;

class Config
{
	private:
		std::vector<t_server>	_servers;
		std::map<int, int>		_server_index;
	public:
		Config();
		Config( const Config & );
		Config &operator=( const Config & );
		~Config();
		bool loadConfig(const std::string &config_file);
		const t_server getServerBlock( int ) const;
};

#endif