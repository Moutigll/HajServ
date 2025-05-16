/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 22:35:11 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/16 06:47:02 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <cerrno>
# include <cstdlib>
# include <cstring>
# include <fcntl.h>
# include <iostream>
# include <map>
# include <netinet/in.h>
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

		const std::string	&getServerName() const;
		const std::string	&getHost() const;
		int					getMaxBodySize() const;
		int					getSocketFd() const;
		int					getPort() const;
		int					getTimeout() const;

	private:
		std::string	_server_name;
		std::string	_host;
		int			_port;
		int			_socket_fd;
		int			_timeout;
		int			_maxBodySize;

		bool	_handleSocketError(const std::string &message);
};

#endif
