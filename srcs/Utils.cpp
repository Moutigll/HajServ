/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 14:29:56 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/25 17:01:58 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Utils.hpp"

std::string trim(const std::string &s)
{
	size_t start = s.find_first_not_of(" \t\r\n");
	size_t end = s.find_last_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	return s.substr(start, end - start + 1);
}

char    back(const std::string &s)
{
	if (s.empty())
		return ('\0');
	return (s[s.length() - 1]);
}

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

std::vector<std::string>	split(const std::string &str, char delim)
{
	std::vector<std::string>	result;
	std::string					token;
	std::istringstream			iss(str);

	while (std::getline(iss, token, delim))
	{
		if (!token.empty())
			result.push_back(token);
	}
	return result;
}

void toLowercase(std::string& s)
{
	for (size_t i = 0; i < s.length(); ++i)
		s[i] = std::tolower(static_cast<unsigned char>(s[i]));
}
