/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetFiles.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 02:22:13 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/26 03:09:03 by ele-lean         ###   ########.fr       */
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

static bool	is_image(const std::string &filename)
{
	const std::string	exts[] = {".png", ".jpg", ".jpeg", ".gif", ".bmp", ".webp", ".svg"};
	size_t				pos = filename.rfind('.');
	if (pos == std::string::npos)
		return false;
	std::string ext = filename.substr(pos);
	for (size_t i = 0; i < sizeof(exts) / sizeof(exts[0]); ++i)
	{
		if (ext == exts[i])
			return true;
	}
	return false;
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

	html = "<html><head><title>Index of " + uri + "</title>\n";
	html += "<style>\n";
	html += "body {\n"
			"	font-family: Arial, sans-serif;\n"
			"	background: #ecf0f1;\n"
			"	padding: 40px;\n"
			"	color: #2c3e50;\n"
			"}\n"
			"h1 {\n"
			"	font-size: 32px;\n"
			"	margin-bottom: 30px;\n"
			"}\n"
			".entry {\n"
			"	display: flex;\n"
			"	align-items: center;\n"
			"	background: #ffffff;\n"
			"	border: 1px solid #ddd;\n"
			"	padding: 12px 16px;\n"
			"	margin-bottom: 12px;\n"
			"	border-radius: 8px;\n"
			"	width: 35%;\n"
			"	transition: all 0.2s ease-in-out;\n"
			"}\n"
			".entry:hover {\n"
			"	background: #f4faff;\n"
			"	box-shadow: 0 2px 8px rgba(0,0,0,0.1);\n"
			"}\n"
			".entry svg.icon, .entry img.thumbnail {\n"
			"	margin-right: 14px;\n"
			"}\n"
			"img.thumbnail {\n"
			"	max-height: 50px;\n"
			"	max-width: 50px;\n"
			"	border: 1px solid #ccc;\n"
			"	border-radius: 4px;\n"
			"}\n"
			"a {\n"
			"	color: #2980b9;\n"
			"	text-decoration: none;\n"
			"	font-size: 16px;\n"
			"}\n"
			"a:hover {\n"
			"	text-decoration: underline;\n"
			"}\n";
	html += "</style>\n";
	html += "</head><body>";
	html += "<div style=\"display: flex; flex-direction: row\"><svg class=\"blahaj\" width=\"128\" height=\"128\" viewBox=\"0 0 570 570\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">\n";
	html += std::string("\t\t") + svgBlahajBase + svgBlahajEye + svgBlahajTummy + svgBlahajShadow + svgBlahajLinearGradients + svgBlahajSmug + svgBlahajTongue + "</svg>\n";
	html += "<h1>Index of " + uri + "</h1></div>";

	while ((entry = readdir(dir)))
	{
		name = std::string(entry->d_name);

		if (name == "." || name[0] == '.')
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
			link += name + "/";

			html += "<div class=\"entry\">";
			html += svgIconFolder;
			html += "<a href=\"" + link + "\">" + name + "/</a>";
			html += "</div>";
		}
		else
		{
			link = uri;
			if (link.empty() || link[link.size() - 1] != '/')
				link += "/";
			link += name;

			html += "<div class=\"entry\">";
			if (is_image(name))
			{
				html += "<a href=\"" + link + "\">";
				html += "<img class=\"thumbnail\" src=\"" + link + "\" alt=\"" + name + "\" />";
				html += name + "</a>";
			}
			else
			{
				html += svgIconFile;
				html += "<a href=\"" + link + "\">" + name + "</a>";
			}
			html += "</div>";
		}
	}
	closedir(dir);

	html += "</body></html>";
	return html;
}
