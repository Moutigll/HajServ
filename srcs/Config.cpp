/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:36:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/30 17:54:20 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/Colors.hpp"

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
		{
			_inserver = true;
			continue ;
		}
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
			parseVectors(line, line_number, server);
			continue ;
		}
		std::istringstream iss(line);
		std::string key, value;

		if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
			std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No value was found." << RESET << std::endl;
		else
		{
			value = value.substr(0, value.length() - 1);
			server._data[key] = value;
		}
	}
	return true;
}


bool	Config::parseVectors(std::string &line, int &line_number, t_servers &server)
{
	std::vector<std::string>* target_vector = NULL;
	if (line.rfind("allowed_methods", 0) == 0)
		target_vector = &server._methods;
	else if (line.rfind("server_name", 0) == 0)
		target_vector = &server._names;
	else if (line.rfind("index", 0) == 0)
		target_vector = &server._indexes;

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