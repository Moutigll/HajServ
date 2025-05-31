/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:36:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/31 13:14:08 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/Colors.hpp"
#include <cstddef>

Config::Config() : _finished(false), _log_connections(false), _log_request(false), _log_level("") {}

Config::Config(Config const &other)
{
	*this = other;
}

Config & Config::operator=(Config const &other)
{
	if (this != &other)
	{
		//
	}
	return *this;
}

Config::~Config() {}

bool Config::parse(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	std::string line;
	int line_number = 0;

	bool _inserver = false;

	if (!file.is_open())
	{
		std::cerr << RED << "Error: Could not open config file: " << filename << RESET << std::endl;
		return false;
	}

	while (std::getline(file, line))
	{
		line_number++;
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue ;
		if (!_inserver && line != "server {")
		{
			if (!parseGlobals(line, line_number))
				return false;
			continue ;
		}
		if (line == "server {")
			_inserver = true;
		if (_inserver)
		{
			if (!parseServer(file, line_number))
				return false;
			_inserver = false;
		}
	}
	_finished = true;
	return true;
}

static std::string getSecondElem(std::string &line)
{
	std::istringstream iss(line);
	std::string value;
	int i = 0;
	while (getline(iss, value, ' '))
	{
		if (i == 1)
			return (value);
		i++;
	}
	return ("");
}

bool	Config::parseServer(std::ifstream &file, int &line_number)
{
	t_servers server;
	std::string line;
	while (std::getline(file, line))
	{
		line_number++;
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue ;
		if (line == "};")
		{
			_servers.push_back(server);
			return true;
		}
		if (line.rfind("allowed_methods", 0) == 0 || line.rfind("server_name", 0) == 0 || line.rfind("index", 0) == 0)
		{
			t_location nullloc;
			parseVectors(line, server, nullloc, false);
			continue ;
		}
		if (line.rfind("location", 0) == 0)
		{
			t_location location;
			location._return_code = 0;
			std::string value = getSecondElem(line);
			if (!value.empty())
				location._path = value;
			parseLocation(file, line_number, location);
			server._locations.push_back(location);
			continue ;
		}
		if (line.rfind("error_pages", 0) == 0)
		{
			std::string value = getSecondElem(line);
			if (!value.empty())
				server._root_error = value;
			parseErrors(file, line_number, server);
			continue ;
		}
		std::istringstream iss(line);
		std::string key, value;

		if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
		{
			std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No value was found." << RESET << std::endl;
			return false;
		}
		value = value.substr(0, value.length() - 1);
		server._data[key] = value;
	}
	std::cerr << RED << "Config file finished without the end of bracket of server which is '};'." << RESET << std::endl;
	return false;
}

bool Config::parseLocation(std::ifstream &file, int &line_number, t_location &loc)
{
	std::string line;
	while (std::getline(file, line))
	{
		line_number++;
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue ;
		if (line == "}")
			return true;
		std::istringstream iss(line);
		if (line.rfind("try_files", 0) == 0 || line.rfind("allowed_methods", 0) == 0)
		{
			t_servers nullserv;
			parseVectors(line, nullserv, loc, true);
			continue ;
		}
		if (line.rfind("autoindex", 0) == 0)
		{
			loc._autoindex = getSecondElem(line) == "on";
			continue ;
		}
		if (line.rfind("return", 0) == 0)
		{
			std::string keyword;
			int code = 0;
			std::string url;

			iss >> keyword;

			if (!(iss >> code) || !(iss >> url) || url == ";")
			{
				std::cerr << RED  << "Error: malformed 'return' directive — missing code or URL at line " << line_number << ": '" << line << "'" << RESET << std::endl;
				return false;
			}
			if (!url.empty() && url[url.length() - 1] == ';')
				url = url.substr(0, url.length() - 1);
			loc._return_code= code;
			loc._return_uri = url;
			continue ;
		}
		std::string key, value;

		if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
			std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No value was found." << RESET << std::endl;
		value = value.substr(0, value.length() - 1);
		loc._loc_data[key] = value;
	}
	std::cerr << RED << "Location parsing finished without the end of bracket of server which is '}'." << RESET << std::endl;
	return false;
}

bool Config::parseErrors(std::ifstream &file, int &line_number, t_servers &server)
{
	std::string line;
	while (std::getline(file, line))
	{
		line_number++;
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue ;
		if (line == "}")
			return true;
		std::istringstream iss(line);
		std::string key, value;

		if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
		{
			std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No value was found." << RESET << std::endl;
			return false;
		}
		value = value.substr(0, value.length() - 1);
		server._errors[atoi(key.c_str())] = value;
	}
	std::cerr << RED << "Errors parsing finished without the end of bracket of server which is '}'." << RESET << std::endl;
	return false;
}

bool	Config::parseVectors(std::string &line, t_servers &server, t_location &loc, bool is_loc)
{
	std::vector<std::string>* target_vector = NULL;

	if (line.rfind("allowed_methods", 0) == 0 && !is_loc)
		target_vector = &server._methods;
	else if (line.rfind("server_name", 0) == 0 && !is_loc)
		target_vector = &server._names;
	else if (line.rfind("index", 0) == 0 && !is_loc)
		target_vector = &server._indexes;
	else if (line.rfind("allowed_methods", 0) == 0 && is_loc)
		target_vector = &loc._methods;
	else if (line.rfind("try_files", 0) == 0 && is_loc)
		target_vector = &loc._try_files;

	std::istringstream iss(line);
	std::string t;
	int i = 0;
	while (getline(iss, t, ' '))
	{
		if (i > 0)
		{
			if (back(t) == ';')
				t = t.substr(0, t.length() - 1);
			target_vector->push_back(t);
		}
		i++;
	}
	return true;
}

bool	Config::parseGlobals(std::string &line, int &line_number)
{
	if (back(line) != ';')
	{
		std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No ';' were found at the end." << RESET << std::endl;
		return false;
	}
	std::istringstream iss(line);
	std::string key, value;

	if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
	{
		std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No value was found." << RESET << std::endl;
		return false;
	}
	value = value.substr(0, value.length() - 1);
	if (key == "log_level")
		_log_level = value;
	if (key == "log_connections")
		_log_connections = (value == "on");
	if (key == "log_requests")
		_log_request = (value == "on");
	return true;
}

const t_servers &Config::getServerBlock(size_t index) const
{
	if (index < this->_servers.size())
		return this->_servers[index];
	static const t_servers empty_server;
	return empty_server;
}

size_t	Config::getServerCount() const
{
	return this->_servers.size();
}