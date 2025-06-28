/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:36:50 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/28 10:51:43 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#include "../includes/Colors.hpp"
#include "../includes/Config.hpp"
#include "../includes/Logger.hpp"
#include "../includes/ServerManager.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << RED << "Usage: " << argv[0] << " <config_file_path>" << RESET << std::endl;
		return 1;
	}

	Config config;
	if (!config.parse(argv[1]))
	{
		std::cerr << RED << "Failed to parse config file." << RESET << std::endl;
		return 1;
	}
	config.logConfig();

	ServerManager serverManager;
	if (!serverManager.init(config.getServers()))
	{
		g_logger.log(LOG_ERROR, "Failed to initialize server manager.");
		return 1;
	}
	serverManager.start();

	return 0;
}
