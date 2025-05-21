/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 23:49:57 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/21 23:55:52 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Utils.hpp"

class Location
{
    private:
        std::string _root;
        std::string _index;
        std::string _cgi_path;
        std::string _cgi_ext;
        bool _autoindex;
    public:
        Location();
        Location( const Location & );
        Location &operator=( const Location & );
        ~Location();

        void setRoot( const std::string &root );
        void setIndex( const std::string &index );
        void setCgiPath( const std::string &cgi_path );
        void setCgiExt( const std::string &cgi_ext );
        void setAutoindex( bool autoindex );

        const std::string &getRoot() const;
        const std::string &getIndex() const;
        const std::string &getCgiPath() const;
        const std::string &getCgiExt() const;
        bool getAutoindex() const;
};

#endif