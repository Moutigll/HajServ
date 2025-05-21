/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:36:50 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/21 15:24:18 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/Colors.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << RED << "Usage: " << argv[0] << " <config_file>" << RESET << std::endl;
        return (1);
    }
    Config config;
    if (!config.loadConfig(argv[1]))
    {
        std::cerr << RED << "Error loading config file." << RESET << std::endl;
        return (1);
    }
    std::cout << GREEN << "Config file loaded successfully." << RESET << std::endl;
    std::cout << CYAN << "Server blocks:" << RESET << std::endl;
    for (size_t i = 0; i < 3; ++i)
    {
        const t_server &server = config.getServerBlock(i);
        std::cout << server._data.at("server_name") << std::endl;
        std::cout << server._error_pages.at(404) << std::endl;
    }
    return (0);
}