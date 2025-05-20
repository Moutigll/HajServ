/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 13:34:08 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/20 11:12:31 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <iostream>

class Location
{
	public:
		Location();
		Location(const Location &src);
		Location &operator=(const Location &src);
		~Location();

		void	setRoot(const std::string &root);
		void	setLocation(const std::string &location);
		void	print() const;

		std::string	getRoot() const;
		std::string	getLocation() const;
	private:
		std::string	_location;
		std::string	_root;
};

#endif