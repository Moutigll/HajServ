/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 23:59:08 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/22 00:16:11 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Location.hpp"

Location::Location()
{
    this->_root = "";
    this->_index = "";
    this->_cgi_path = "";
    this->_cgi_ext = "";
    this->_autoindex = false;
}

Location::Location(const Location &other)
{
    *this = other;
}

Location &Location::operator=(const Location &other)
{
    if (this != &other)
    {
        this->_root = other._root;
        this->_index = other._index;
        this->_cgi_path = other._cgi_path;
        this->_cgi_ext = other._cgi_ext;
        this->_autoindex = other._autoindex;
    }
    return *this;
}

Location::~Location() { }

void Location::setRoot(const std::string &root)
{
    this->_root = root;
}

void Location::setIndex(const std::string &index)
{
    this->_index = index;
}

void Location::setCgiPath(const std::string &cgi_path)
{
    this->_cgi_path = cgi_path;
}

void Location::setCgiExt(const std::string &cgi_ext)
{
    this->_cgi_ext = cgi_ext;
}

void Location::setAutoindex(bool autoindex)
{
    this->_autoindex = autoindex;
}

const std::string &Location::getRoot() const
{
    return this->_root;
}

const std::string &Location::getIndex() const
{
    return this->_index;
}

const std::string &Location::getCgiPath() const
{
    return this->_cgi_path;
}

const std::string &Location::getCgiExt() const
{
    return this->_cgi_ext;
}

bool Location::getAutoindex() const
{
    return this->_autoindex;
}