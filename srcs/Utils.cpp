/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 14:29:56 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/02 20:06:42 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Utils.hpp"

/**
 * @brief Trims leading and trailing whitespace from a string.
 *
 * Removes spaces, tabs, carriage returns, and newline characters
 * from the beginning and end of the given string.
 *
 * @param[in] s The string to trim.
 * @return std::string A new string with whitespace removed from both ends.
 */
std::string trim(const std::string &s)
{
	size_t start = s.find_first_not_of(" \t\r\n");
	size_t end = s.find_last_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	return s.substr(start, end - start + 1);
}

/**
 * @brief Returns the last character of a string.
 *
 * If the string is empty, returns the null character '\0'.
 *
 * @param[in] s The string to extract the last character from.
 * @return char The last character of the string, or '\0' if the string is empty.
 */
char    back(const std::string &s)
{
    if (s.empty())
		return ('\0');
	return (s[s.length() - 1]);
}

/**
 * @brief Gets the current timestamp as a formatted string with millisecond precision.
 *
 * The format of the returned string is: YYYY-MM-DD HH:MM:SS.mmm
 * (e.g., "2025-06-02 14:23:58.127").
 *
 * @return std::string The current local time with milliseconds, formatted as a string.
 */
std::string	getTimestamp(void)
{
	struct timeval	tv;
	struct tm		tm;
	char			buf[24];
	int				millisec;
	std::ostringstream	oss;

	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm);
	millisec = tv.tv_usec / 1000;
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
	oss << buf << '.' << (millisec < 100 ? (millisec < 10 ? "00" : "0") : "") << millisec;
	return (oss.str());
}