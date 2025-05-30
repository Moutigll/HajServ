/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:36:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/30 15:04:34 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/Colors.hpp"

Config::Config() : _finished(false) {}

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
	bool _inserver = false;
	std::ifstream file(filename.c_str());
	std::string line;
	int line_number = 0;

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
			GetGlobals(line, line_number);
			continue ;
		}
		if (line == "server {")
			_inserver = true;
		std::cout << line << std::endl;
	}
	return true;
}

bool	Config::GetGlobals(std::string &line, int &line_number)
{
	if (line.empty())
		return true;
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