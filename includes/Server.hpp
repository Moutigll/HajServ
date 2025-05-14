/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 22:35:11 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/15 01:28:06 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <cerrno>
# include <cstdlib>
# include <cstring>
# include <fcntl.h>
# include <map>
# include <iostream>
# include <netinet/in.h>
# include <string>
# include <sys/socket.h>
# include <unistd.h>

# include "Colors.hpp"

class Server
{
	public:
		Server();
		Server(const std::map<std::string, std::string> &config);
		Server(const Server &src);
		Server &operator=(const Server &src);
		~Server();

		bool	setupSocket();
		void	start();

		const std::string	&getHost() const;
		int					getPort() const;
		const std::string	&getServerName() const;
		int					getSocketFd() const;

	private:
		std::string	_host;
		int			_port;
		std::string	_server_name;
		int			_socket_fd;

		bool	_handleSocketError(const std::string &message);
};

#endif
