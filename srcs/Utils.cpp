/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 22:15:37 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/16 05:43:16 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Utils.hpp"

volatile sig_atomic_t	g_stop = 0;
int						g_pipe_fds[2];

void signalHandler(int signum)
{
	char c = 0;
	ssize_t ret = write(g_pipe_fds[1], &c, 1);
	(void)ret;
	if (signum == SIGINT || signum == SIGTERM)
		g_stop = 1;
}

void setupSignalHandler()
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signalHandler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	signal(SIGPIPE, SIG_IGN);

	if (sigaction(SIGINT, &sa, NULL) == -1)
		std::cerr << "Error setting up SIGINT handler: " << strerror(errno) << std::endl;

	if (sigaction(SIGTERM, &sa, NULL) == -1)
		std::cerr << "Error setting up SIGTERM handler: " << strerror(errno) << std::endl;
}

std::string trim(const std::string &s)
{
	size_t start = s.find_first_not_of(" \t\r\n");
	size_t end = s.find_last_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	return s.substr(start, end - start + 1);
}

std::string	toLower(std::string str)
{
	size_t	i;

	i = 0;
	while (i < str.size())
	{
		str[i] = std::tolower(str[i]);
		++i;
	}
	return (str);
}
/*
* Format RFC1123 : "Day, DD Mon YYYY HH:MM:SS GMT"
*/
std::string getHttpDate()
{
	char buf[100];
	std::time_t t = std::time(0);
	std::tm *gmt = std::gmtime(&t);

	if (std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt))
		return std::string(buf);
	return std::string();
}