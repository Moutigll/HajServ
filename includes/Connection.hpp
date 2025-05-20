/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/15 22:45:38 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/20 11:48:52 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Request.hpp"
# include "Response.hpp"
# include "Server.hpp"

class Connection
{
	public:
		Connection(int fd, Server *server, const sockaddr_in &clientAddr);
		~Connection();

		bool	readRequest();
		bool	writeResponse();

		bool	isClosed() const;
		int		getFd() const;
		bool	isRequestComplete() const;
		bool	isResponseBuilt() const { return _responseBuilt; }
		time_t	getLastActivity() const { return _lastActivity; }
		Server	*getServer() const;

		void	resetForNextRequest();
	private:
		int			_fd;
		Server		*_server;

		Request		_request;
		Response	_response;

		bool		_isKeepAlive;
		bool		_responseBuilt;
		bool		_requestComplete;
		time_t		_lastActivity;
		std::string	_rawRequest;
		std::string	_rawResponse;
		size_t		_bytesWritten;
		sockaddr_in	_clientAddr;

		bool		parseRequest(const std::string &raw);
		void		generateResponse();
};

#endif
