/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 23:23:37 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/20 11:19:24 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include <poll.h>
# include <vector>

# include "Connection.hpp"
# include "Config.hpp"
# include "Server.hpp"
# include "Utils.hpp"

class ServerManager
{
	public:
		ServerManager();
		ServerManager(const ServerManager &src);
		ServerManager &operator=(const ServerManager &src);
		~ServerManager();

		void	addServer(const t_server &server);
		void	startServers();

	private:
		std::vector<Server>			_servers;
		std::vector<struct pollfd>	_pollFds;
		std::map<int, Connection*>	_connections;

		void	cleanupClosedConnections();
		Server*	findServerByFd(int fd);
		bool	isListeningSocket(int fd) const;
		void	runPollLoop();
		void	cleanup();
		void	handleIoEvents();
		void	acceptNewConnections(int listenFd);
		void	markConnectionClosed(int fd, size_t index);
		void	checkTimeouts();
};

#endif
