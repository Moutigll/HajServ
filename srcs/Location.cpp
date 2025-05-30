/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 23:59:08 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/30 14:19:22 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Location.hpp"

Location::Location()
{
    //
}

Location::Location(const Location &other)
{
    *this = other;
}

Location &Location::operator=(const Location &other)
{
    if (this != &other)
    {
        //
    }
    return *this;
}

Location::~Location() { }
