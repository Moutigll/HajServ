/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:36:47 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/23 04:36:16 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/Colors.hpp"

#include <sstream>
#include <cctype>
#include <cstdlib>

const std::vector<t_server>& Config::getServers() const {
	return servers;
}

const t_server& Config::getServer(size_t index) const {
	return servers.at(index);
}

size_t Config::getServerCount() const {
	return servers.size();
}

void Config::syntaxError(const std::string &msg, int line) const {
	std::cerr << "Syntax Error (line " << line << "): " << msg << std::endl;
}

std::vector<Config::Token> Config::tokenize(std::ifstream &file) {
	std::vector<Token> tokens;
	std::string line;
	int line_number = 0;

	while (std::getline(file, line)) {
		++line_number;
		size_t i = 0;
		while (i < line.size()) {
			while (i < line.size() && std::isspace(line[i])) i++;
			if (i >= line.size() || line[i] == '#') break;

			if (line[i] == '{' || line[i] == '}' || line[i] == ';') {
				tokens.push_back((Token){std::string(1, line[i++]), line_number});
			} else {
				size_t start = i;
				while (i < line.size() && !std::isspace(line[i]) && line[i] != ';' && line[i] != '{' && line[i] != '}') i++;
				tokens.push_back((Token){line.substr(start, i - start), line_number});
			}
		}
	}
	return tokens;
}

bool Config::parse(const std::string &filename) {
	std::ifstream file(filename.c_str());
	if (!file.is_open()) {
		syntaxError("Cannot open file", 0);
		return false;
	}

	std::vector<Token> tokens = tokenize(file);
	size_t i = 0;
	while (i < tokens.size()) {
		if (tokens[i].value == "server") {
			t_server server;
			++i;
			if (i >= tokens.size() || tokens[i].value != "{") {
				syntaxError("Expected '{' after server", tokens[i - 1].line);
				return false;
			}
			++i;
			if (!parseServerBlock(tokens, i, server)) return false;
			servers.push_back(server);
		} else {
			syntaxError("Expected 'server' block", tokens[i].line);
			return false;
		}
	}
	return true;
}

bool Config::parseServerBlock(std::vector<Token> &tokens, size_t &i, t_server &server) {
	while (i < tokens.size() && tokens[i].value != "}") {
		if (tokens[i].value == "location") {
			t_location location;
			++i;
			if (i >= tokens.size()) return false;
			location.path = tokens[i++].value;
			if (i >= tokens.size() || tokens[i].value != "{") return false;
			++i;
			if (!parseLocationBlock(tokens, i, location)) return false;
			server.locations.push_back(location);
		} else if (tokens[i].value == "error_pages") {
			++i;
			if (!parseErrorPagesBlock(tokens, i, server.error_pages)) return false;
		} else {
			if (!parseDirective(tokens, i, "server", server, NULL)) return false;
		}
	}
	if (i >= tokens.size() || tokens[i].value != "}") return false;
	++i;
	return true;
}

bool Config::parseLocationBlock(std::vector<Token> &tokens, size_t &i, t_location &location) {
	while (i < tokens.size() && tokens[i].value != "}") {
		if (!parseDirective(tokens, i, "location", *(t_server*)NULL, &location)) return false;
	}
	if (i >= tokens.size() || tokens[i].value != "}") return false;
	++i;
	return true;
}

bool Config::parseErrorPagesBlock(std::vector<Token> &tokens, size_t &i, std::map<int, std::string> &map) {
	if (i >= tokens.size() || tokens[i].value != "{") return false;
	++i;
	while (i + 2 < tokens.size() && tokens[i + 2].value == ";") {
		int code = std::atoi(tokens[i].value.c_str());
		std::string page = tokens[i + 1].value;
		map[code] = page;
		i += 3;
	}
	if (i >= tokens.size() || tokens[i].value != "}") return false;
	++i;
	return true;
}

bool Config::parseDirective(std::vector<Token> &tokens, size_t &i, const std::string &context, t_server &server, t_location *location) {
	std::string key = tokens[i++].value;
	std::vector<std::string> values;
	while (i < tokens.size() && tokens[i].value != ";") {
		values.push_back(tokens[i++].value);
	}
	if (i >= tokens.size() || tokens[i].value != ";") return false;
	++i;

	if (context == "server") {
		if (key == "server_name") server.server_name = values;
		else if (key == "listen") server.listen = values[0];
		else if (key == "timeout") server.timeout = std::atoi(values[0].c_str());
		else if (key == "allowed_methods") server.allowed_methods = values;
		else if (key == "root") server.root = values[0];
		else if (key == "index") server.index = values;
		else if (key == "client_max_body_size") server.client_max_body_size = std::atol(values[0].c_str());
		else if (key == "access_log") server.access_log = values[0];
		else if (key == "error_log") server.error_log = values[0];
		else return false;
	} else if (context == "location") {
		if (key == "root") location->root = values[0];
		else if (key == "index") location->index = values;
		else if (key == "alias") location->alias = values[0];
		else if (key == "try_files") location->try_files = values;
		else if (key == "return") {
			location->return_code = std::atoi(values[0].c_str());
			if (values.size() > 1)
				location->return_url = values[1];
		}
		else if (key == "autoindex") location->autoindex = (values[0] == "on");
		else if (key == "allowed_methods") location->allowed_methods = values;
		else if (key == "cgi_pass") location->cgi_pass = values[0];
		else return false;
	}
	return true;
}
