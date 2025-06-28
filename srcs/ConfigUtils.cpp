/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/28 08:20:22 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/28 08:27:27 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/Logger.hpp"

bool Config::defaultingServer(t_server &server)
{
	if (server._hosts.empty())
	{
		server._hosts.push_back("localhost");
		std::cout << YELLOW << "Warning: No server_name directive found. Defaulting to 'localhost'." << RESET << std::endl;
	}
	if (server._ports.empty())
	{
		server._ports.push_back(80);
		std::cout << YELLOW << "Warning: No listen directive found. Defaulting to port 80." << RESET << std::endl;
	}
	if (server._methods.empty())
	{
		server._methods.push_back("GET");
		server._methods.push_back("POST");
		server._methods.push_back("DELETE");
		std::cout << YELLOW << "Warning: No allowed_methods directive found. Defaulting to GET, POST, DELETE." << RESET << std::endl;
	}
	for (unsigned long i = 0; i < server._locations.size(); i++)
	{
		if (server._locations[i]._methods.empty())
		{
			std::cout << YELLOW << "Warning: Location '" << server._locations[i]._path << "' has no methods directive. Defaulting to general methods." << RESET << std::endl;
			for (unsigned long j = 0; j < server._methods.size(); j++)
				server._locations[i]._methods.push_back(server._methods[j]);
		}
		if (server._locations[i]._root.empty() && server._root.empty())
		{
			std::cerr << RED << "Error: Server '" << i + 1 << "' is missing root directive." << RESET << std::endl;
			return false;
		}
		else if (server._locations[i]._root.empty())
			server._locations[i]._root = server._root;
	}
	return true;
}

int Config::parseServerKeys(std::istringstream &iss, int &line_number, std::string &line, t_server &server)
{
	std::string key, value;
	
	if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
	{
		std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No value was found." << RESET << std::endl;
		return false;
	}
	if (key == "timeout")
	{
		size_t size = atoll(value.c_str());
		if (size <= 0)
		{
			std::cerr << RED << "Error: Invalid value '" << value << "' at line " << line_number << "." << RESET << std::endl;
			return false;
		}
		if (key == "timeout")
			server._timeout = size;
		return -1;
	}
	if (key == "max_body_size" || key == "client_max_body_size")
	{
		size_t* target = "max_body_size" == key ? &server._maxBodySize : &server._clientMaxBodySize;
		*target = giveBodySize(value);
		if (*target == 0)
		{
			std::cerr << RED << "Error: Invalid max_body_size value '" << value << "' at line " << line_number << "." << RESET << std::endl;
			return false;
		}
		return -1;
	}
	if (key == "root")
	{
		if (!server._root.empty())
		{
			std::cerr << RED << "Multiple root detected in server parsing at " << line_number << "." << RESET << std::endl;
			return false;
		}
		server._root = value;
		return -1;
	}
	server._data[key] = value;
	return true;
}

std::vector<std::string> *getTargetVector(const std::string &line, bool is_loc, t_server &server, t_location &loc, bool &is_port)
{
	is_port = false;

	std::map<std::string, std::vector<std::string> *> map;

	if (is_loc)
		map["allowed_methods"] = &loc._methods;
	else
	{
		map["allowed_methods"] = &server._methods;
		map["server_name"] = &server._hosts;
		map["index"] = &server._indexes;

		if (line.rfind("listen", 0) == 0)
		{
			is_port = true;
			return NULL;
		}
	}

	for (std::map<std::string, std::vector<std::string> *>::iterator it = map.begin(); it != map.end(); ++it)
		if (line.rfind(it->first, 0) == 0)
			return it->second;

	return NULL;
}

bool Config::parseVectors(std::string &line, t_server &server, t_location &loc, bool is_loc)
{
	bool is_port = false;

	std::vector<std::string> *target_vector = getTargetVector(line, is_loc, server, loc, is_port);

	std::istringstream iss(line);
	std::string t;
	int i = 0;
	while (getline(iss, t, ' '))
	{
		if (i > 0)
		{
			if (is_port)
			{
				int port = atoi(t.c_str());
				if (port <= 0 || port > 65535)
				{
					std::cerr << RED << "Error: Invalid port number '" << t << std::endl;
					return false;
				}
				server._ports.push_back(port);
				continue;
			}

			target_vector->push_back(t);
		}
		i++;
	}
	return true;
}

int Config::parseLocationKeys(std::istringstream &iss, int &line_number, std::string &line, t_location &loc)
{
	std::string key, value;

	if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
		std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No value was found." << RESET << std::endl;

	if (key == "root")
	{
		if (!loc._root.empty())
		{
			std::cerr << RED << "Multiple root detected in location parsing at " << line_number << "." << RESET << std::endl;
			return false;
		}
		loc._root = value;
		return -1;
	}
	if (key == "index")
	{
		loc._indexes.push_back(value);
		return -1;
	}
	if ((loc._cgi_timeout = atoi(value.c_str())) <= 0)
	{
		std::cerr << RED << "Error: Invalid line '" << line << "' at line " << line_number << "." << RESET << std::endl;
		return false;
	}
	return true;
}

int Config::goToOtherParse(std::ifstream &file, int &line_number, std::string &line, t_server &server)
{
	if (line.rfind("location", 0) == 0)
	{
		t_location location;
		std::string value = getSecondElem(line);
		if (!value.empty())
			location._path = value;
		if (!parseLocation(file, line_number, location))
			return false;
		if (location._path.empty())
		{
			std::cerr << RED << "Error: Location parsing error." << RESET << std::endl;
			return false;
		}
		if (location._indexes.empty())
			location._indexes.push_back("");
		server._locations.push_back(location);
		return -1;
	}
	if (line.rfind("error_pages", 0) == 0)
	{
		std::string value = getSecondElem(line);
		if (!value.empty())
			server._root_error = value;
		if (!parseErrors(file, line_number, server))
			return false;
		return -1;
	}
	return true;
}
