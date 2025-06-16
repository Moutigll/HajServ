/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 23:50:18 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/03 20:23:41 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <cstdlib>
# include <ctime>
# include <fstream>
# include <iostream>
# include <map>
# include <sstream>
# include <string>
# include <sys/time.h>
# include <vector>

/**
 * @brief Trims leading and trailing whitespace from a string.
 *
 * Removes spaces, tabs, carriage returns, and newline characters
 * from the beginning and end of the given string.
 *
 * @param[in] s The string to trim.
 * @return std::string A new string with whitespace removed from both ends.
 */
std::string	trim(const std::string &s);

/**
 * @brief Returns the last character of a string.
 *
 * If the string is empty, returns the null character '\0'.
 *
 * @param[in] s The string to extract the last character from.
 * @return char The last character of the string, or '\0' if the string is empty.
 */
char		back(const std::string &s);

/**
 * @brief Gets the current timestamp as a formatted string with millisecond precision.
 *
 * The format of the returned string is: YYYY-MM-DD HH:MM:SS.mmm
 * (e.g., "2025-06-02 14:23:58.127").
 *
 * @return std::string The current local time with milliseconds, formatted as a string.
 */
std::string	getTimestamp(void);

/**
 * @brief Splits a string into a vector of substrings based on a delimiter.
 *
 * This function takes a string and splits it into parts using the specified
 * delimiter character. Empty tokens are ignored.
 *
 * @param[in] str The string to split.
 * @param[in] delim The delimiter character used to split the string.
 * @return std::vector<std::string> A vector containing the substrings.
 */
std::vector<std::string>	split(const std::string &str, char delim);

template <typename T>
std::string	to_string(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return (oss.str());
}

#endif