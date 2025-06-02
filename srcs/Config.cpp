/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:36:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/02 20:00:37 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"

Config::Config()
: _finished(false), _log_connections(false), _log_request(false), _log_console(false), _log_file(""), _log_level("none")
{}

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

/* --------------------
 *       Parsing
 * -------------------- */

/**
 * @brief Parses the configuration file.
 *
 * Opens the file, reads it line by line, and parses global
 * directives and server blocks.
 *
 * @param filename The path to the config file.
 * @return true if parsing was successful, false otherwise.
 */
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
	g_logger.init(_log_file, _log_level == "debug" ? LOG_DEBUG :
					_log_level == "info" ? LOG_INFO :
					_log_level == "warning" ? LOG_WARNING :
					_log_level == "error" ? LOG_ERROR : LOG_CRITICAL,
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

/**
 * @brief Parses global directives outside of server blocks.
 *
 * Currently supports logging directives such as log_connections,
 * log_request, log_console, log_file, and log_level.
 *
 * @param line The config line to parse.
 * @param line_number The current line number for error messages.
 * @return true if the global directive was parsed successfully, false otherwise.
 */
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

/**
 * @brief Parses a server block from the config file.
 *
 * Reads lines until the server block end '};' is found.
 * Parses server-level directives and nested location blocks.
 *
 * @param file The opened config file stream.
 * @param line_number Reference to the current line number (updated as parsing proceeds).
 * @return true if the server block was parsed successfully, false on error.
 */
bool	Config::parseServer(std::ifstream &file, int &line_number)
{
	t_server server;
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

bool	Config::parseVectors(std::string &line, t_server &server, t_location &loc, bool is_loc)
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

/**
 * @brief Parses a location block inside a server.
 *
 * Reads lines until the location block end '}' is found.
 * Parses directives related to this location.
 *
 * @param file The opened config file stream.
 * @param line_number Reference to the current line number.
 * @param loc The location struct to fill with parsed data.
 * @return true if the location block was parsed successfully, false otherwise.
 */
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
			t_server nullserv;
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

/**
 * @brief Parses error pages block inside a server.
 *
 * Reads lines until '}' is found. Maps HTTP error codes to error page paths.
 *
 * @param file The opened config file stream.
 * @param line_number Reference to the current line number.
 * @param server The server struct to update with error pages.
 * @return true if error pages block parsed successfully, false otherwise.
 */
bool Config::parseErrors(std::ifstream &file, int &line_number, t_server &server)
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

/* --------------------
 *       Getters
 * -------------------- */

const t_server &Config::getServerBlock(size_t index) const
{
	if (index < this->_servers.size() && index >= 0)
		return this->_servers[index];
	static const t_server empty_server;
	return empty_server;
}

size_t	Config::getServerCount() const
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

static void	logVectorStr(std::string &out, const std::string &label,
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

template<typename K, typename V>
static void	logMapStr(std::string &out, const std::string &label,
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

static void	printLocationStr(std::string &out, const t_location &loc, size_t index)
{
	out += std::string(BRIGHT_PURPLE "\tLocation [") + to_string(index) + "]" RESET + "\n";
	out += std::string("\t\t") + BRIGHT_CYAN "Path: " RESET + BRIGHT_WHITE + loc._path + RESET + "\n";
	out += std::string("\t\t") + BRIGHT_CYAN "Autoindex: " RESET + (loc._autoindex ? BRIGHT_GREEN "on" RESET : BRIGHT_RED "off" RESET) + "\n";

	if (loc._return_code != 0)
	{
		out += std::string("\t\t") + BRIGHT_CYAN "Return: " RESET +
			to_string(loc._return_code) + " " + BRIGHT_MAGENTA + loc._return_uri + RESET + "\n";
	}

	logVectorStr(out, "Methods", loc._methods, CYAN);
	logVectorStr(out, "Try files", loc._try_files, BRIGHT_GREEN);

	logMapStr(out, "Additional data", loc._loc_data, BRIGHT_WHITE, BRIGHT_YELLOW);
}

void	Config::logConfig() const
{
	std::string	out;

	out += BRIGHT_CYAN "\nGlobal settings:" RESET "\n";
	out += std::string("\tlog_level: ") + BRIGHT_WHITE + _log_level + RESET + "\n";
	out += std::string("\tlog_connections: ") + (_log_connections ? BRIGHT_GREEN "on" RESET : BRIGHT_RED "off" RESET) + "\n";
	out += std::string("\tlog_requests: ") + (_log_request ? BRIGHT_GREEN "on" RESET : BRIGHT_RED "off" RESET) + "\n\n";

	for (size_t i = 0; i < _servers.size(); i++)
	{
		const t_server &srv = _servers[i];

		out += std::string(BRIGHT_PURPLE "Server block [") + to_string(i) + "]" RESET + "\n";

		logVectorStr(out, "Methods", srv._methods, CYAN);
		logVectorStr(out, "Server names", srv._names, BRIGHT_CYAN);
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
