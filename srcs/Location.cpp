/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 13:36:12 by ele-lean          #+#    #+#             */
/*   Updated: 2025/05/19 15:55:15 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Location.hpp"

Location::Location() : _root("") {}
Location::Location(const Location &src) : _root(src._root) {}
Location &Location::operator=(const Location &src)
{
	if (this != &src)
		_root = src._root;
	return *this;
}
Location::~Location() {}

void Location::setRoot(const std::string &root)
{
	_root = root;
}

void Location::setLocation(const std::string &location)
{
	_location = location;
}

std::string Location::getRoot() const
{
	return _root;
}

std::string Location::getLocation() const
{
	return _location;
}