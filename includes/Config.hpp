/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 21:20:52 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/19 15:58:29 by ele-lean         ###   ########.fr       */
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


# include "Colors.hpp"
# include "Location.hpp"
# include "Utils.hpp"


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

		const std::string							&getGlobal(const std::string &key) const;
		const std::map<std::string, std::string>	&getServerBlock(size_t index) const;

		size_t	getServerCount() const;
	private:
		bool	parseServerBlock(std::ifstream &file, int &line_number);
		bool	parseLocationBlock(std::ifstream &file, int &line_number, Location &location);

		bool												_loaded;
		std::map<std::string, std::string>					_globals;
		std::vector<std::map<std::string, std::string> >	_servers;
};

#endif
