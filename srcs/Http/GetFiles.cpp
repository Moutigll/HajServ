/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetFiles.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 02:22:13 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/28 22:31:09 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iomanip>

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
	envMap["REQUEST_METHOD"] = _method;
	envMap["SCRIPT_FILENAME"] = filepath;
	envMap["SCRIPT_NAME"] = _filePath;
	envMap["SERVER_SOFTWARE"] = VERSION;

	if (!_query.empty())
		envMap["QUERY_STRING"] = _query;
	else
		envMap["QUERY_STRING"] = "";

	// Content
	// if (_method == "POST")
	// {
	// 	envMap["CONTENT_LENGTH"] = "0";
	// 	envMap["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
	// }
	// else
	envMap["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
	envMap["CONTENT_LENGTH"] = "0";

	// Additional headers
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		std::string key = it->first;
		for (size_t i = 0; i < key.length(); ++i)
			if (key[i] == '-') key[i] = '_';

		envMap["HTTP_" + key] = it->second;
	}
}

static std::string	uriEncode(const std::string &value)
{
	std::ostringstream	escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (size_t i = 0; i < value.size(); ++i)
	{
		unsigned char c = value[i];

		// Alphanumeric characters and a few special characters are not encoded
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/')
			escaped << c;
		else
			escaped << '%' << std::uppercase << std::setw(2) << int(c) << std::nouppercase;
	}
	return escaped.str();
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

	html = "<html><head><meta charset=\"UTF-8\"><title>Index of " + uri + "</title>\n";
	html += indexPageBase;

	html += std::string("\t\t") + svgBlahajBase + svgBlahajEye + svgBlahajTummy + svgBlahajShadow + svgBlahajLinearGradients + svgBlahajSmug + svgBlahajTongue + "</svg>\n";
	html += "<h1>Index of " + uri + "</h1></div>";

	// Add parent directory link if not at root
	if (uri != "/")
	{
		std::string parent_uri = uri;
		if (parent_uri[parent_uri.size() - 1] == '/')
			parent_uri.erase(parent_uri.size() - 1);

		std::size_t last_slash = parent_uri.find_last_of('/');
		if (last_slash != std::string::npos)
			parent_uri = parent_uri.substr(0, last_slash + 1);
		else
			parent_uri = "/";

		html += "<div class=\"entry\">";
		html += svgIconFolder;
		html += "<a href=\"" + uriEncode(parent_uri) + "\">../</a>";
		html += "</div>";
	}
	while ((entry = readdir(dir))) // Read each entry in the directory
	{
		name = std::string(entry->d_name);
		if (name == "." || name[0] == '.') // Skip current and hidden files
			continue;

		full_path = directory_path;
		if (full_path.empty() || full_path[full_path.size() - 1] != '/')
			full_path += "/";
		full_path += uriEncode(name);

		if (stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) // If it's a directory
		{
			link = uri;
			if (link.empty() || link[link.size() - 1] != '/')
				link += "/";
			link += uriEncode(name);
			if (link[link.size() - 1] != '/')
				link += "/"; 

			html += "<div class=\"entry\">";
			html += svgIconFolder;
			html += "<a href=\"" + link + "\">" + name + "</a>";
			html += "</div>";
		} else {
			link = uri;
			if (link.empty() || link[link.size() - 1] != '/')
				link += "/";
			link += uriEncode(name);

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
