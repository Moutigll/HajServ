/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:36:50 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/23 04:36:01 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/Colors.hpp"

void printLocation(const t_location& loc) {
	std::cout << "  Location Path: " << loc.path << "\n";
	if (!loc.root.empty()) std::cout << "    Root: " << loc.root << "\n";
	if (!loc.alias.empty()) std::cout << "    Alias: " << loc.alias << "\n";
	if (!loc.index.empty()) {
		std::cout << "    Index: ";
		for (size_t j = 0; j < loc.index.size(); ++j)
			std::cout << loc.index[j] << " ";
		std::cout << "\n";
	}
	if (!loc.try_files.empty()) {
		std::cout << "    Try Files: ";
		for (size_t j = 0; j < loc.try_files.size(); ++j)
			std::cout << loc.try_files[j] << " ";
		std::cout << "\n";
	}
	if (loc.return_code)
		std::cout << "    Return: " << loc.return_code << " " << loc.return_url << "\n";
	std::cout << "    Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
	if (!loc.allowed_methods.empty()) {
		std::cout << "    Allowed Methods: ";
		for (size_t j = 0; j < loc.allowed_methods.size(); ++j)
			std::cout << loc.allowed_methods[j] << " ";
		std::cout << "\n";
	}
	if (!loc.cgi_pass.empty())
		std::cout << "    CGI Pass: " << loc.cgi_pass << "\n";
}

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <config_file_path>\n";
		return 1;
	}

	Config config;
	if (!config.parse(argv[1])) {
		std::cerr << "Failed to parse config file.\n";
		return 1;
	}

	const std::vector<t_server>& servers = config.getServers();
	for (size_t i = 0; i < servers.size(); ++i) {
		const t_server& server = servers[i];
		std::cout << "Server " << i + 1 << ":\n";
		std::cout << "  Server Names: ";
		for (size_t j = 0; j < server.server_name.size(); ++j)
			std::cout << server.server_name[j] << " ";
		std::cout << "\n";
		std::cout << "  Listen: " << server.listen << "\n";
		std::cout << "  Timeout: " << server.timeout << "\n";
		if (!server.allowed_methods.empty()) {
			std::cout << "  Allowed Methods: ";
			for (size_t j = 0; j < server.allowed_methods.size(); ++j)
				std::cout << server.allowed_methods[j] << " ";
			std::cout << "\n";
		}
		if (!server.error_pages.empty()) {
			std::cout << "  Error Pages:\n";
			for (std::map<int, std::string>::const_iterator it = server.error_pages.begin(); it != server.error_pages.end(); ++it)
				std::cout << "    " << it->first << " => " << it->second << "\n";
		}
		if (!server.root.empty())
			std::cout << "  Root: " << server.root << "\n";
		if (!server.index.empty()) {
			std::cout << "  Index: ";
			for (size_t j = 0; j < server.index.size(); ++j)
				std::cout << server.index[j] << " ";
			std::cout << "\n";
		}
		if (server.client_max_body_size > 0)
			std::cout << "  Client Max Body Size: " << server.client_max_body_size << "\n";
		if (!server.access_log.empty())
			std::cout << "  Access Log: " << server.access_log << "\n";
		if (!server.error_log.empty())
			std::cout << "  Error Log: " << server.error_log << "\n";

		if (!server.locations.empty()) {
			std::cout << "  Locations:\n";
			for (size_t j = 0; j < server.locations.size(); ++j)
				printLocation(server.locations[j]);
		}
		std::cout << "\n";
	}

	return 0;
}