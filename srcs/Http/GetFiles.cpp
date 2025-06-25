/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetFiles.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 02:22:13 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/25 23:29:33 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/GetFiles.hpp"

t_location *GetCgi(t_server *server, const std::string &uri)
{
	if (!server)
		return NULL;

	for (size_t i = 0; i < server->_locations.size(); ++i)
	{
		t_location &loc = server->_locations[i];
		const std::string &path = loc._path;

		// We only handle regex locations ending with '$'
		if (path.empty() || path[path.size() - 1] != '$')
			continue; // skip this location

		size_t dollarPos = path.size() - 1;
		size_t dotPos = path.rfind('.', dollarPos);
		if (dotPos == std::string::npos || dotPos >= dollarPos)
			continue; // no extension found, skip

		std::string ext = path.substr(dotPos, dollarPos - dotPos); // extract like ".py"

		// Check if URI ends with this extension
		if (uri.size() >= ext.size())
		{
			if (uri.compare(uri.size() - ext.size(), ext.size(), ext) == 0)
			{
				// Also confirm location has cgi_pass configured
				std::map<std::string, std::string>::const_iterator it = loc._loc_data.find("cgi_root");
				if (it != loc._loc_data.end() && !it->second.empty())
					return &loc;
			}
		}
	}

	return NULL; // no matching location found
}

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

std::string	generateAutoindexPage(const std::string &uri, const std::string &directory_path)
{
	DIR				*dir;
	struct dirent	*entry;
	std::string		html;
	struct stat		st;
	std::string		full_path;
	std::string		link;
	std::string		name;

	dir = opendir(directory_path.c_str());
	if (!dir)
		return "<html><body><h1>500 Internal Server Error</h1></body></html>";

	html = "<html><head><title>Index of " + uri + "</title></head><body>";
	html += "<h1>Index of " + uri + "</h1><ul>";

	while ((entry = readdir(dir)))
	{
		name = std::string(entry->d_name);

		// Ignore hidden files and "." entry
		if (name == ".")
			continue;

		full_path = directory_path;
		if (full_path.empty() || full_path[full_path.size() - 1] != '/')
			full_path += "/";
		full_path += name;

		if (stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
		{
			link = uri;
			if (link.empty() || link[link.size() - 1] != '/')
				link += "/";
			link += name;
			link += "/"; // add trailing slash for directories

			html += "<li><a href=\"" + link + "\">" + name + "/</a></li>";
		}
		else
		{
			link = uri;
			if (link.empty() || link[link.size() - 1] != '/')
				link += "/";
			link += name;

			html += "<li><a href=\"" + link + "\">" + name + "</a></li>";
		}
	}
	closedir(dir);

	html += "</ul></body></html>";
	return html;
}
