/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 18:30:04 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/28 10:22:58 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#include "../includes/Colors.hpp"
#include "../includes/Logger.hpp"
#include "../includes/Utils.hpp"

Logger g_logger; // Global logger instance

Logger::Logger(void)
: m_level(LOG_INFO), m_to_console(true)
{}

Logger::~Logger(void)
{
	close();
}

/*
 * @brief Removes ANSI color codes from a string.
 *
 * This function scans the input string and removes any ANSI escape sequences
 * used for coloring text in the console, returning a clean string without colors.
 *
 * @param str The input string potentially containing ANSI color codes.
 * @return std::string The cleaned string without color codes.
 */
static std::string	removeColors(const std::string &str)
{
	std::string	result;
	size_t		i = 0;

	while (i < str.size())
	{
		if (str[i] == '\033' && i + 1 < str.size() && str[i + 1] == '[')
		{
			i += 2;
			while (i < str.size() && str[i] != 'm')
				i++;
			if (i < str.size())
				i++;
		}
		else
		{
			result += str[i];
			i++;
		}
	}
	return (result);
}

void	Logger::init(const std::string &file_path, e_log_level level, bool to_console)
{
	m_level = level;
	m_to_console = to_console;
	if (!file_path.empty())
		m_log_file.open(file_path.c_str(), std::ios::app);
}

void	Logger::close(void)
{
	if (m_log_file.is_open())
		m_log_file.close();
}

std::string	Logger::levelToString(e_log_level level) const
{
	switch (level)
	{
		case LOG_DEBUG:
			return (CYAN " [DEBUG] " RESET);
		case LOG_INFO:
			return (GREEN " [INFO] " RESET);
		case LOG_WARNING:
			return (YELLOW " [WARNING] " RESET);
		case LOG_ERROR:
			return (RED " [ERROR] " RESET);
		case LOG_CRITICAL:
			return (BRIGHT_RED " [CRITICAL] " RESET);
		default:
			return (WHITE " [UNKNOWN] " RESET);
	}
}

void	Logger::log(e_log_level level, const std::string &msg)
{
	if (level < m_level)
		return ;

	std::string output_console = getTimestamp() + levelToString(level) + msg + "\n";
	if (m_to_console)
		std::cout << output_console;

	if (m_log_file.is_open())
	{
		std::string output_file = getTimestamp() + removeColors(levelToString(level)) + removeColors(msg) + "\n";
		m_log_file << output_file;
	}
}
