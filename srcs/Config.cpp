/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:36:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/26 22:49:30 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/Logger.hpp"
#include <iostream>

Config::Config()
	: _finished(false), _log_connections(false), _log_request(false), _log_console(false), _log_file(""), _log_level("none"), _servers()
{
}

Config::Config(Config const &other)
{
	*this = other;
}

Config &Config::operator=(Config const &other)
{
	if (this != &other)
	{
		this->_finished = other._finished;
		this->_log_connections = other._log_connections;
		this->_log_request = other._log_request;
		this->_log_console = other._log_console;
		this->_log_file = other._log_file;
		this->_log_level = other._log_level;
		this->_servers = other._servers;
	}
	return *this;
}

Config::~Config() {}

/* --------------------
 *       Parsing
 * -------------------- */

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
			continue;
		if (!_inserver && line != "server {")
		{
			if (!parseGlobals(line, line_number))
				return false;
			continue;
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
	g_logger.init(_log_file, _log_level == "debug" ? LOG_DEBUG : _log_level == "info"  ? LOG_INFO
															 : _log_level == "warning" ? LOG_WARNING
															 : _log_level == "error"   ? LOG_ERROR
																					   : LOG_CRITICAL,
				  _log_console);
	_finished = true;
	return true;
}

/**
 * @brief Extract the second element from a whitespace-separated string.
 *
 * This function parses the input string `line` by splitting it at spaces
 * and returns the second element found. If the second element does not exist,
 * it returns an empty string.
 *
 * @param[in] line Reference to the input string to parse.
 * @return std::string The second element in the string, or an empty string if none.
 */
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

bool Config::parseGlobals(std::string &line, int &line_number)
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
	value = value.substr(0, value.length() - 1); // remove ';'

	if (key == "log_level")
		_log_level = value;
	else if (key == "log_connections")
		_log_connections = (value == "on");
	else if (key == "log_requests")
		_log_request = (value == "on");
	else if (key == "log_console")
		_log_console = (value == "on");
	else if (key == "log_file")
		_log_file = value;

	return true;
}

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

bool Config::parseServer(std::ifstream &file, int &line_number)
{
	t_server server;

	std::string line;
	while (std::getline(file, line))
	{
		line_number++;
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		if (line == "};")
		{
			if (!defaultingServer(server))
				return false;
			_servers.push_back(server);
			return true;
		}
		if (line.rfind("allowed_methods", 0) == 0 || line.rfind("server_name", 0) == 0 || line.rfind("index", 0) == 0 || line.rfind("listen", 0) == 0)
		{
			t_location nullloc;
			if (!parseVectors(line, server, nullloc, false))
				return false;
			continue;
		}
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
			continue;
		}
		if (line.rfind("error_pages", 0) == 0)
		{
			std::string value = getSecondElem(line);
			if (!value.empty())
				server._root_error = value;
			parseErrors(file, line_number, server);
			continue;
		}
		std::istringstream iss(line);
		std::string key, value;

		if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
		{
			std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No value was found." << RESET << std::endl;
			return false;
		}

		value = value.substr(0, value.length() - 1);

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
			continue;
		}
		if (key == "max_body_size")
		{
			size_t size = atoll(value.c_str());
			if (size <= 0)
			{
				std::cerr << RED << "Error: Invalid value '" << value << "' at line " << line_number << "." << RESET << std::endl;
				return false;
			}
			if (value[value.length() - 1] == 'G')
				server._maxBodySize = size * 1024 * 1024 * 1024; // Convert GB to bytes
			else if (value[value.length() - 1] == 'M')
				server._maxBodySize = size * 1024 * 1024; // Convert MB to bytes
			else if (value[value.length() - 1] == 'K')
				server._maxBodySize = size * 1024; // Convert KB to bytes
			else
				server._maxBodySize = size; // Default case, assume bytes
			continue;
		}
		if (key == "root")
		{
			if (!server._root.empty())
			{
				std::cerr << RED << "Multiple root detected in server parsing at " << line_number << "." << RESET << std::endl;
				return false;
			}
			server._root = value;
			continue;
		}

		server._data[key] = value;
	}
	std::cerr << RED << "Config file finished without the end of bracket of server which is '};'." << RESET << std::endl;
	return false;
}

std::vector<std::string> *getTargetVector(const std::string &line, bool is_loc, t_server &server, t_location &loc, bool &is_port)
{
	is_port = false;

	std::map<std::string, std::vector<std::string> *> map;

	if (is_loc)
	{
		map["allowed_methods"] = &loc._methods;
		map["try_files"] = &loc._try_files;
	}
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
			if (back(t) == ';')
				t = t.substr(0, t.length() - 1);
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

bool Config::parseLocation(std::ifstream &file, int &line_number, t_location &loc)
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
		if (line.rfind("try_files", 0) == 0 || line.rfind("allowed_methods", 0) == 0)
		{
			t_server nullserv;
			parseVectors(line, nullserv, loc, true);
			continue;
		}
		if (line.rfind("autoindex", 0) == 0)
		{
			loc._autoindex = getSecondElem(line) == "on;";
			continue;
		}
		if (line.rfind("return", 0) == 0)
		{
			std::string keyword;
			int code = 0;
			std::string url;

			iss >> keyword;

			if (!(iss >> code) || !(iss >> url) || url == ";")
			{
				std::cerr << RED << "Error: malformed 'return' directive — missing code or URL at line " << line_number << ": '" << line << "'" << RESET << std::endl;
				return false;
			}
			if (!url.empty() && url[url.length() - 1] == ';')
				url = url.substr(0, url.length() - 1);
			loc._return_code = code;
			loc._return_uri = url;
			continue;
		}
		if (line.rfind("fastcgi_pass", 0) == 0)
		{
			std::string keyword;
			std::string command, extension;

			iss >> keyword;

			if (!(iss >> command >> extension) || (iss >> std::ws && !iss.eof()))
			{
				std::cerr << RED << "Error: malformed 'return' directive — missing code or URL at line " << line_number << ": " << line << "" << RESET << std::endl;
				return false;
			}
			if (!extension.empty() && extension[extension.length() - 1] == ';')
				extension = extension.substr(0, extension.length() - 1);
			loc._cgi[extension] = command;
			continue;
		}
		std::string key, value;

		if (!(iss >> key >> value) || (iss >> std::ws && !iss.eof()))
			std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No value was found." << RESET << std::endl;

		if (value[value.length() -1] == ';')
			value = value.substr(0, value.length() - 1);
		if (key == "root")
		{
			if (!loc._root.empty())
			{
				std::cerr << RED << "Multiple root detected in location parsing at " << line_number << "." << RESET << std::endl;
				return false;
			}
			loc._root = value;
			continue;
		}
		if (key == "index")
		{
			loc._indexes.push_back(value);
			continue;
		}
		loc._cgi_timeout = atoi(value.c_str());
	}
	std::cerr << RED << "Location parsing finished without the end of bracket of server which is '}'." << RESET << std::endl;
	return false;
}

bool Config::parseErrors(std::ifstream &file, int &line_number, t_server &server)
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
			std::cerr << RED << "Syntax error at line " << line_number << ": '" << line << "'. No value was found." << RESET << std::endl;
			return false;
		}
		value = value.substr(0, value.length() - 1);
		server._errors[atoi(key.c_str())] = value;
	}
	std::cerr << RED << "Errors parsing finished without the end of bracket of server which is '}'." << RESET << std::endl;
	return false;
}

/* --------------------
 *       Getters
 * -------------------- */

const t_server &Config::getServerBlock(size_t index) const
{
	if (index < this->_servers.size())
		return this->_servers[index];
	return this->_servers[0];
}

size_t Config::getServerCount() const
{
	return this->_servers.size();
}

/* --------------------
 *       Logging
 * --------------------*/

/**
 * @brief Logs the configuration to the logger.
 *
 * Formats the configuration data into a human-readable string
 * and sends it to the logger.
 */

static void logVectorStr(std::string &out, const std::string &label,
						 const std::vector<std::string> &vec, const char *color)
{
	out += std::string(BRIGHT_YELLOW "\t" + label + ": " RESET);
	for (size_t i = 0; i < vec.size(); i++)
	{
		out += std::string(color) + vec[i] + RESET;
		if (i + 1 < vec.size())
			out += std::string(BRIGHT_WHITE ", " RESET);
	}
	out += "\n";
}

template <typename K, typename V>
static void logMapStr(std::string &out, const std::string &label,
					  const std::map<K, V> &mapData,
					  const char *keyColor, const char *valColor)
{
	out += std::string(BRIGHT_BLUE "\t" + label + ":" RESET) + "\n";
	for (typename std::map<K, V>::const_iterator it = mapData.begin(); it != mapData.end(); ++it)
	{
		out += std::string("\t\t") + keyColor + to_string(it->first) + RESET + ": " +
			   valColor + to_string(it->second) + RESET + "\n";
	}
}

static void printLocationStr(std::string &out, const t_location &loc, size_t index)
{
	out += std::string(BRIGHT_PURPLE "\tLocation [") + to_string(index) + "]" RESET + "\n";
	out += std::string("\t\t") + BRIGHT_CYAN "Path: " RESET + BRIGHT_WHITE + loc._path + RESET + "\n";
	out += std::string("\t\t") + BRIGHT_CYAN "Autoindex: " RESET + (loc._autoindex ? BRIGHT_GREEN "on" RESET : BRIGHT_RED "off" RESET) + "\n";
	out += std::string("\t\t") + BRIGHT_CYAN "Root: " RESET + BRIGHT_WHITE + loc._root + RESET + "\n";
	out += std::string("\t\t") + BRIGHT_CYAN "Cgi Timeout: " RESET + BRIGHT_WHITE + to_string(loc._cgi_timeout) + RESET + "\n";

	if (loc._return_code != 0)
	{
		out += std::string("\t\t") + BRIGHT_CYAN "Return: " RESET +
			   to_string(loc._return_code) + " " + BRIGHT_MAGENTA + loc._return_uri + RESET + "\n";
	}

	logVectorStr(out, "Methods", loc._methods, CYAN);
	logVectorStr(out, "Try files", loc._try_files, BRIGHT_GREEN);
	logVectorStr(out, "Indexes", loc._indexes, BRIGHT_GREEN);

	logMapStr(out, "Cgi", loc._cgi, BRIGHT_WHITE, BRIGHT_YELLOW);
}

void Config::logConfig() const
{
	std::string out;

	out += BRIGHT_CYAN "\nGlobal settings:" RESET "\n";
	out += std::string("\tlog_level: ") + BRIGHT_WHITE + _log_level + RESET + "\n";
	out += std::string("\tlog_connections: ") + (_log_connections ? BRIGHT_GREEN "on" RESET : BRIGHT_RED "off" RESET) + "\n";
	out += std::string("\tlog_requests: ") + (_log_request ? BRIGHT_GREEN "on" RESET : BRIGHT_RED "off" RESET) + "\n\n";

	for (size_t i = 0; i < _servers.size(); i++)
	{
		const t_server &srv = _servers[i];

		out += std::string(BRIGHT_PURPLE "Server block [") + to_string(i) + "]" RESET + "\n";

		logVectorStr(out, "Methods", srv._methods, CYAN);
		logVectorStr(out, "Server names", srv._hosts, BRIGHT_CYAN);
		out += BRIGHT_YELLOW + std::string("\tMax body size: ") + BRIGHT_CYAN + to_string(srv._maxBodySize) + " bytes" RESET + "\n";
		out += BRIGHT_YELLOW + std::string("\tTimeout: ") + BRIGHT_CYAN + to_string(srv._timeout) + " s" RESET + "\n";
		out += std::string(BRIGHT_BLUE "\tPorts (") + to_string(srv._ports.size()) + "):" RESET + "\n";
		for (size_t j = 0; j < srv._ports.size(); j++)
			out += std::string("\t\t") + BRIGHT_YELLOW + to_string(srv._ports[j]) + RESET + "\n";
		logVectorStr(out, "Indexes", srv._indexes, BRIGHT_GREEN);

		out += std::string(BRIGHT_YELLOW "\tRoot error page: " RESET) + BRIGHT_RED + srv._root_error + RESET + "\n";

		logMapStr(out, "Data key-values", srv._data, BRIGHT_WHITE, BRIGHT_YELLOW);
		logMapStr(out, "Error pages", srv._errors, BRIGHT_RED, BRIGHT_GREEN);

		out += std::string(BRIGHT_BLUE "\tLocations (") + to_string(srv._locations.size()) + "):" RESET + "\n";

		for (size_t j = 0; j < srv._locations.size(); j++)
			printLocationStr(out, srv._locations[j], j);

		out += "\n";
	}

	g_logger.log(LOG_DEBUG, out);
}

const std::vector<t_server> &Config::getServers() const
{
	return this->_servers;
}
