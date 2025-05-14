/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 21:21:00 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/15 01:33:05 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/ServerManager.hpp"

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

	Config	cfg;
	if (!cfg.load(path))
	{
		std::cerr << RED << "Failed to load config file: " << path << RESET << std::endl;
		return 1;
	}
	std::cout << GREEN << "Config file loaded successfully!" << RESET << std::endl;
	ServerManager	manager;

	for (size_t i = 0; i < cfg.getServerCount(); i++)
		manager.addServer(cfg.getServerBlock(i));

	manager.startServers();

	return 0;
}