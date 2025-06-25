/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetFiles.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 02:24:15 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/25 16:38:08 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GETFILES_HPP
# define GETFILES_HPP

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
 * @brief Strips the location prefix from a request URI.
 * 
 * This function removes the specified location path from the beginning of the request URI,
 * allowing for easier handling of requests that match a specific location.
 * 
 * @param request the full request URI
 * @param location_path the path of the location to strip
 * @return std::string the request URI with the location path stripped
 */
std::string	stripLocationPrefix(std::string const &request, std::string const &location_path);

/**
 * @brief Joins a root path with a relative path.
 * 		This function concatenates a root directory with a relative path and
 * 		removes any redundant slashes.
 * 
 * @param root the root directory path
 * @param path the relative path to join with the root
 * @return std::string the combined path, ensuring no double slashes
 */
std::string	joinPaths(std::string const &root, std::string const &path);

#endif
