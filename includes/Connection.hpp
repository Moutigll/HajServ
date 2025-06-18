/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 18:48:52 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/03 18:49:07 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include <string>
# include <vector>
# include <unistd.h>

class Connection
{
public:
	Connection(void);
	Connection(int fd);
	Connection(const Connection &other);
	Connection &operator=(const Connection &other);
	~Connection(void);

	int		getFd(void) const;
	bool	isClosed(void) const;

	bool	handleRead(void);
	bool	handleWrite(void);

private:
	int			_fd;
	bool		_closed;
	std::string	_readBuffer;
	std::string	_writeBuffer;

	bool		parseRequest(void);
	void		generateResponse(void);
};

#endif