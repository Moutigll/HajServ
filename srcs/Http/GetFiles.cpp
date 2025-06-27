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
#include "../../includes/Http/HttpResponse.hpp"

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

bool	checkMethod(const std::string &method, const std::vector<std::string> &allowedMethods)
{
	for (size_t i = 0; i < allowedMethods.size(); ++i)
	{
		if (method == allowedMethods[i])
			return (true);
	}
	return (false);
}

static bool	isImage(const std::string &filename)
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

void HttpResponse::generateEnvMap(const std::string &filepath, std::map<std::string, std::string> &envMap)
{
	std::vector<std::string> envVec;

	envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	envMap["SERVER_PROTOCOL"] = _protocol;
	envMap["REQUEST_METHOD"] = _uri;
	envMap["SCRIPT_FILENAME"] = filepath;
	envMap["SCRIPT_NAME"] = _filePath;
	envMap["SERVER_SOFTWARE"] = VERSION;

	// Query string
	size_t pos = _uri.find('?');
	if (pos != std::string::npos)
		envMap["QUERY_STRING"] = _uri.substr(pos + 1);
	else
		envMap["QUERY_STRING"] = "";

	// Content
	if (_method == "POST")
	{
		envMap["CONTENT_LENGTH"] = to_string(0);
		envMap["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
	}
	else
		envMap["CONTENT_LENGTH"] = "0";

	// Additional headers
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		std::string key = it->first;
		for (size_t i = 0; i < key.length(); ++i)
			if (key[i] == '-') key[i] = '_';

		envMap["HTTP_" + key] = it->second;
	}

	// Convert map to envVec
	envMap.clear();
	for (std::map<std::string, std::string>::const_iterator it = envMap.begin(); it != envMap.end(); ++it) {
		envVec.push_back(it->first + "=" + it->second);
	}
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
	html += indexPageBase;

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
			if (isImage(name))
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
