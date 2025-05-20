/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 22:35:11 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/20 14:47:15 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <arpa/inet.h>
# include <cerrno>
# include <cstdlib>
# include <cstring>
# include <fcntl.h>
# include <iostream>
# include <map>
# include <netinet/in.h>
# include <unistd.h>
#include <fstream>
#include <sstream>

# include "Config.hpp"
# include "Response.hpp"

class Server
{
	public:
		Server();
		Server(const t_server &server);
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
		void				logConnection(int client_fd, const sockaddr_in &client_addr, int mode);

		const Location	*matchLocation(const std::string &uri) const;
		// Methods to handle requests and responses
		void				get(Response &response, const std::string &path);

	private:
		std::string				_server_name;
		std::string				_host;
		int						_port;
		int						_socket_fd;
		int						_timeout;
		int						_maxBodySize;
		std::vector<Location>	_locations;

		bool	_handleSocketError(const std::string &message);
};

#endif
