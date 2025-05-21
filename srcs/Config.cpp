/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:36:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/22 00:15:44 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/Colors.hpp"
#include <cstdlib>

Config::Config() { }

Config::Config(const Config &other)
{
	*this = other;
}

Config &Config::operator=(const Config &other)
{
	if (this != &other)
	{
		this->_servers = other._servers;
		this->_server_index = other._server_index;
	}
	return *this;
}

Config::~Config()
{
	this->_servers.clear();
	this->_server_index.clear();
}

bool Config::loadConfig(const std::string &config_file)
{
	std::ifstream file(config_file.c_str());
	if (!file.is_open())
	{
		std::cerr << RED << "Error opening config file: " << config_file << RESET << std::endl;
		return false;
	}
	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty() || line[0] == '#')
			continue ;
		std::cout << CYAN << "Parsing line: " << line << RESET << std::endl;
	}
	return true;
}

const t_server Config::getServerBlock(int index) const
{
	if (index < 0 || index >= static_cast<int>(_servers.size()))
		throw std::out_of_range("Index out of range");
	return _servers[index];
}