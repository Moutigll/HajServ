/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 21:21:00 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/14 22:09:27 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << YELLOW << "Usage: " << argv[0] << " <config_file>" << RESET << std::endl;
		return 1;
	}

	Config cfg;

	if (!cfg.load(argv[1]))
	{
		return 1;
	}

	std::cout << GREEN << "Config file loaded successfully!" << RESET << std::endl;
	cfg.state();

	return 0;
}
