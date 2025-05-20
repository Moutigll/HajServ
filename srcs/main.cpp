/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 21:21:00 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/20 11:41:02 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/ServerManager.hpp"

Config	g_config;

int	main(int argc, char **argv)
{
	const char	*path;

	if (pipe(g_pipe_fds) == -1)
	{
		std::cerr << "Failed to create pipe: " << strerror(errno) << std::endl;
		return 1;
	}

	setupSignalHandler();

	if (argc > 2)
	{
		std::cerr << YELLOW << "Usage: " << argv[0] << " <config_file>" << RESET << std::endl;
		return 1;
	}

	if (argc == 2)
		path = argv[1];
	else
		path = "default.conf";

	if (!g_config.load(path))
	{
		std::cerr << RED << "Failed to load config file: " << path << RESET << std::endl;
		return 1;
	}
	std::cout << GREEN << "Config file loaded successfully!" << RESET << std::endl;
	if (g_config.getGlobal("log_level") == "debug")
		g_config.state();
	ServerManager	manager;

	for (size_t i = 0; i < g_config.getServerCount(); i++)
		manager.addServer(g_config.getServerBlock(i));

	manager.startServers();

	return 0;
}