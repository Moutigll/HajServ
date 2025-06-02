/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 23:50:18 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/02 19:37:03 by ele-lean         ###   ########.fr       */
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

std::string	trim(const std::string &s);
char		back(const std::string &s);
std::string	getTimestamp(void);

template <typename T>
std::string	to_string(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return (oss.str());
}

#endif