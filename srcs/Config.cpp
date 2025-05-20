/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 21:26:46 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/20 11:17:28 by ele-lean         ###   ########.fr       */
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
	std::cout << GREEN << "[ Global Config ]" << RESET << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = _globals.begin(); it != _globals.end(); ++it)
		std::cout << it->first << ": " << it->second << std::endl;

	std::cout << GREEN << "\n[ Server Blocks ]" << RESET << std::endl;
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		std::cout << YELLOW << "Server " << i << RESET << std::endl;
		for (std::map<std::string, std::string>::const_iterator it = _servers[i]._data.begin(); it != _servers[i]._data.end(); ++it)
			std::cout << "  " << it->first << ": " << it->second << std::endl;

		const std::vector<Location> &locations = _servers[i]._locations;
		for (size_t j = 0; j < locations.size(); ++j)
			locations[j].print();
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

const t_server &Config::getServerBlock(size_t index) const
{
	if (index < this->_servers.size())
		return this->_servers[index];
	static const t_server empty_server;
	return empty_server;
}

size_t	Config::getServerCount() const
{
	return this->_servers.size();
}

bool	Config::parseServerBlock(std::ifstream &file, int &line_number)
{
	std::string line;
	t_server server;

	while (std::getline(file, line))
	{
		line_number++;
		line = trim(line);

		if (line.empty() || line[0] == '#')
			continue;

		if (line == "}")
		{
			this->_servers.push_back(server);
			return true;
		}
		else if (line.rfind("location", 0) == 0)
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

			server._locations.push_back(location);
			continue;
		}
		else if (line == "server {")
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
		server._data[key] = value;
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
