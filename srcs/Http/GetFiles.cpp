/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetFiles.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 02:22:13 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/25 16:36:57 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/GetFiles.hpp"

t_location	*findBestLocation(t_server *server, const std::string &uri)
{
	t_location				*best_location;
	std::string				best_match;
	std::vector<t_location>::iterator	it;

	best_location = NULL;
	best_match = "";

	it = server->_locations.begin();
	while (it != server->_locations.end())
	{
		if (uri.find(it->_path) == 0)
		{
			if (it->_path.length() > best_match.length())
			{
				best_match = it->_path;
				best_location = &(*it);
			}
		}
		++it;
	}
	return (best_location);
}

bool	checkMethod(const std::string &method, const std::vector<std::string> &allowed_methods)
{
	for (size_t i = 0; i < allowed_methods.size(); ++i)
	{
		if (method == allowed_methods[i])
			return (true);
	}
	return (false);
}

std::string	stripLocationPrefix(std::string const &request, std::string const &location_path)
{
	size_t loc_len = location_path.size();
	if (location_path[loc_len - 1] == '/')
		loc_len -= 1;

	if (request.compare(0, loc_len, location_path, 0, loc_len) == 0)
		return request.substr(loc_len);

	return request; // fallback
}


std::string	joinPaths(std::string const &root, std::string const &path)
{
	std::string	result;
	size_t		root_len;
	size_t		path_start;

	root_len = root.size();
	path_start = 0;

	if (root_len > 0 && root[root_len - 1] == '/')
		result = root.substr(0, root_len - 1);
	else
		result = root;

	while (path_start < path.size() && path[path_start] == '/')
		path_start++;

	result += "/";
	result += path.substr(path_start);

	return result;
}
