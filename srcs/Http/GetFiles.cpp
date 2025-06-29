/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetFiles.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 02:22:13 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/29 05:41:21 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iomanip>

#include "../../includes/Http/GetFiles.hpp"
#include "../../includes/Http/HttpResponse.hpp"
#include <fstream>
#include <iostream>

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
	size_t pos = filepath.find_last_of('/');
	envMap["SCRIPT_NAME"] = (pos == std::string::npos) ? filepath : filepath.substr(pos + 1);
	envMap["SERVER_SOFTWARE"] = VERSION;

	if (!_query.empty())
		envMap["QUERY_STRING"] = _query;
	else
		envMap["QUERY_STRING"] = "";
	
	envMap["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
	envMap["CONTENT_LENGTH"] = "0";
	if (_method == "POST" && !_requestBody.empty()) {
        if (_requestHeaders.count("Content-Type"))
            envMap["CONTENT_TYPE"] = _requestHeaders["Content-Type"];
        if (_requestHeaders.count("Content-Length"))
			envMap["CONTENT_LENGTH"] = _requestHeaders["Content-Length"];
    }

	// Additional headers
	for (std::map<std::string,std::string>::const_iterator it = _requestHeaders.begin();
         it != _requestHeaders.end(); ++it)
    {
        const std::string &key = it->first;
        const std::string &val = it->second;

		std::cout << "key: " << key << " val: " << val << std::endl;

        if (key == "Cookie")
		{
            envMap["HTTP_COOKIE"] = val;

            size_t pos = 0;
            while (pos < val.size())
			{
                size_t semi = val.find(';', pos);
                std::string pair = val.substr(pos, (semi==std::string::npos ? std::string::npos : semi-pos));
                if (!pair.empty() && pair[0]==' ') pair.erase(0,1);
                size_t eq = pair.find('=');
                if (eq != std::string::npos)
				{
                    std::string cname = pair.substr(0, eq);
                    std::string cval  = pair.substr(eq+1);
                    envMap[cname] = cval;
                }
                if (semi == std::string::npos) break;
                pos = semi + 1;
            }
            continue;
        }

        // Other headers: HTTP_<NAME>
        std::string hkey = key;
        for (size_t i = 0; i < hkey.length(); ++i)
            if (hkey[i] == '-') hkey[i] = '_';
        envMap["HTTP_" + hkey] = val;
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

int	deleteFile(const std::string &filePath)
{
	struct stat	s_stat;
	int			ret;

	ret = stat(filePath.c_str(), &s_stat);
	if (ret != 0)
		return 204; // Not Found

	if (!S_ISREG(s_stat.st_mode))
		return 403; // Forbidden, not a regular file

	ret = access(filePath.c_str(), W_OK);
	if (ret != 0)
		return 403; // Forbidden, file not writable

	ret = unlink(filePath.c_str());
	if (ret != 0)
		return 500; // Internal Server Error, could not delete file

	return 200; // OK, file deleted successfully
}

int postFile(const std::string &body, const std::string &target_path)
{
	struct stat st;

	if (stat(target_path.c_str(), &st) == 0)
		return 409; // Conflict (file already exists)


	std::ofstream out(target_path.c_str(), std::ios::binary);
	if (!out.is_open())
		return 500; // Internal Server Error

	out.write(body.c_str(), body.size());
	if (!out.good()) {
		out.close();
		unlink(target_path.c_str()); // Cleanup if write failed
		return 500;
	}

	out.close();
	return 201; // Created
}

