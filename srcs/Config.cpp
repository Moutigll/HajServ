/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 21:26:46 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/19 17:46:23 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"

Config::Config() : _loaded(false) {}

Config::Config(const Config &src)
{
	*this = src;
}

Config &Config::operator=(const Config &src)
{
	if (this != &src)
	{
		this->_globals = src._globals;
		this->_servers = src._servers;
		this->_loaded = src._loaded;
	}
	return *this;
}

Config::~Config() {}

bool	Config::load(const std::string &path)
{
	std::ifstream file(path.c_str());
	std::string line;
	int line_number = 0;

	if (!file.is_open())
	{
		std::cerr << RED << "Error: Could not open config file: " << path << RESET << std::endl;
		return false;
	}

	while (std::getline(file, line))
	{
		line_number++;
		line = trim(line);

		if (line.empty() || line[0] == '#')
			continue;

		if (line == "server {")
		{
			if (!parseServerBlock(file, line_number))
				return false;
		}
		else
		{
			std::istringstream iss(line);
			std::string key, value;

			if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
			{
				std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'" << RESET << std::endl;
				return false;
			}
			if (!value.empty() && value[value.length() - 1] == ';')
				value = value.substr(0, value.length() - 1);
			this->_globals[key] = value;
		}
	}
	this->_loaded = true;
	return true;
}

bool	Config::isLoaded() const
{
	return this->_loaded;
}

void	Config::state() const
{
	std::map<std::string, std::string>::const_iterator it = this->_globals.begin();
	while (it != this->_globals.end())
	{
		std::cout << it->first << ": " << it->second << std::endl;
		it++;
	}

	for (size_t i = 0; i < this->_servers.size(); i++)
	{
		std::cout << std::endl << "Server block " << i << ":\n";
		std::map<std::string, std::string>::const_iterator it2 = this->_servers[i].begin();
		while (it2 != this->_servers[i].end())
		{
			std::cout << it2->first << ": " << it2->second << std::endl;
			it2++;
		}
	}
}

const std::string &Config::getGlobal(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator it = this->_globals.find(key);
	if (it != this->_globals.end())
		return it->second;
	static const std::string empty = "";
	return empty;
}

const std::map<std::string, std::string> &Config::getServerBlock(size_t index) const
{
	if (index < this->_servers.size())
		return this->_servers[index];
	static const std::map<std::string, std::string> empty_map;
	return empty_map;
}

size_t	Config::getServerCount() const
{
	return this->_servers.size();
}

bool	Config::parseServerBlock(std::ifstream &file, int &line_number)
{
	std::string line;
	std::map<std::string, std::string> server_block;

	while (std::getline(file, line))
	{
		line_number++;
		line = trim(line);

		if (line.empty() || line[0] == '#')
			continue;

		if (line == "}")
		{
			this->_servers.push_back(server_block);
			return true;
		} else if (line.rfind("location", 0) == 0)
		{
			Location location;

			std::istringstream iss(line);
			std::string keyword, path;

			if (!(iss >> keyword >> path))
			{
				std::cerr << RED << "Syntax error: invalid location line at " << line_number << ": '" << line << "'" << RESET << std::endl;
				return false;
			}

			if (!path.empty() && path[path.length() - 1] == '{')
				path = path.substr(0, path.length() - 1);

			location.setLocation(path);

			if (!parseLocationBlock(file, line_number, location))
				return false;
			continue;
		} else if (line == "server {")
		{
			std::cerr << RED << "Nested server block at line " << line_number << ": '" << line << "'" << RESET << std::endl;
			return false;
		}

		std::istringstream iss(line);
		std::string key, value;

		if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
		{
			std::cerr << RED << "Syntax error in server block at line " << line_number << ": '" << line << "'" << RESET << std::endl;
			return false;
		}
		if (!value.empty() && value[value.length() - 1] == ';')
			value = value.substr(0, value.length() - 1);
		server_block[key] = value;
	}

	std::cerr << RED << "Error: server block not closed properly before EOF" << RESET << std::endl;
	return false;
}

bool	Config::parseLocationBlock(std::ifstream &file, int &line_number, Location &location)
{
	std::string line;

	while (std::getline(file, line))
	{
		line_number++;
		line = trim(line);

		if (line.empty() || line[0] == '#')
			continue;

		if (line == "}")
			return true;

		std::istringstream iss(line);
		std::string key, value;

		if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
		{
			std::cerr << RED << "Syntax error in location block at line " << line_number << ": '" << line << "'" << RESET << std::endl;
			return false;
		}
		if (!value.empty() && value[value.length() - 1] == ';')
			value = value.substr(0, value.length() - 1);
		if (key == "root")
			location.setRoot(value);
	}

	std::cerr << RED << "Error: location block not closed properly before EOF" << RESET << std::endl;
	return false;
}
