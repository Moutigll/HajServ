/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 14:29:56 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/23 23:17:38 by etaquet          ###   ########.fr       */
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

std::string percentDecode(const std::string& encoded)
{
    std::ostringstream decoded;
    size_t i = 0;

    while (i < encoded.length())
	{
        if (encoded[i] == '%')
		{
            if (i + 2 >= encoded.length())
                throw std::runtime_error("Invalid percent-encoding (truncated)");

            std::string hexStr = encoded.substr(i + 1, 2);
            int byteInt = 0;

            std::istringstream hexStream(hexStr);
            hexStream >> std::hex >> byteInt;

            if (hexStream.fail())
                throw std::runtime_error("Invalid hex in percent-encoding: " + hexStr);

            decoded << static_cast<char>(byteInt);
            i += 3;
        }
		else if (encoded[i] == '+')
		{
            decoded << ' '; // '+' means space in query strings
            i++;
        }
		else
		{
            decoded << encoded[i];
            i++;
        }
    }

    return decoded.str();
}

void toLowercase(std::string& s)
{
    for (size_t i = 0; i < s.length(); ++i)
        s[i] = std::tolower(static_cast<unsigned char>(s[i]));
}