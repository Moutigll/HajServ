/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetFiles.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 02:24:15 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/25 19:49:35 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GETFILES_HPP
# define GETFILES_HPP

# include <dirent.h>

# include "../Utils.hpp"

/**
 * @brief Finds the best matching location for a given URI in a server's configuration.
 * 
 * @param server the server configuration to search within
 * @param uri the URI to match against the server's locations
 * @return t_location* pointer to the best matching location, or NULL if none found
 */
t_location	*findBestLocation(t_server *server, const std::string &uri);

/**
 * @brief Checks if the given HTTP method is allowed for the specified location.
 * 
 * @param method the HTTP method to check (e.g., "GET", "POST")
 * @param allowed_methods a vector of allowed methods for the location
 * @return
 */
bool		checkMethod(const std::string &method, const std::vector<std::string> &allowed_methods);

/**
 * @brief Generates an autoindex HTML page for a directory.
 * This function creates an HTML page listing the contents of a directory,
 * including links to files and subdirectories.
 * @param uri the URI of the directory being indexed
 * @param directory_path the path to the directory to index
 * @return std::string an HTML string representing the autoindex page
 * 
 */
std::string	generateAutoindexPage(const std::string &uri, const std::string &directory_path);

#endif
