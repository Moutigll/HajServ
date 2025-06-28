/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:40:42 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/29 01:13:44 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/GetFiles.hpp"
#include "../../includes/Logger.hpp"
#include "../../includes/Http/HttpResponse.hpp"

HttpResponse::HttpResponse(const t_server &server)
	: HttpTransaction(),
	  _server(server),
	  _response(),
	  _filePath(),
	  _readFd(-1),
	  _isHeadersSent(false),
	  _ErrorStatus(),
	  _isCgiComplete(true)
{
	_status = 200;
	_protocol = "HTTP/1.1";
	_isComplete = false;
}

HttpResponse::HttpResponse(const t_server &server, HttpRequest &request)
	: 
	  _server(server),
	  _response(),
	  _filePath(),
	  _readFd(-1),
	  _isHeadersSent(false),
	  _ErrorStatus(),
	  _cgiHandler(NULL),
	  _isCgiComplete(true)
{
	this->_method = request.getMethod();
	this->_uri = request.getRequest();
	this->_query = request.getQuery();
	this->_protocol = request.getProtocol();
	this->_connectionKeepAlive = request.isConnectionKeepAlive();
	this->_headers.clear(); // Clear headers from the request, we will build our own response headers
}

HttpResponse::HttpResponse(const HttpResponse &other)
	: HttpTransaction(other),
	  _server(other._server),
	  _response(other._response),
	  _filePath(other._filePath),
	  _readFd(other._readFd),
	  _isHeadersSent(other._isHeadersSent),
	  _ErrorStatus(other._ErrorStatus),
	  _cgiHandler(other._cgiHandler)
{}

HttpResponse &HttpResponse::operator=(const HttpResponse &other) {
	if (this != &other) {
		HttpTransaction::operator=(other);
		_server = other._server;
		_isHeadersSent = other._isHeadersSent;
		_readFd = other._readFd;
		_response = other._response;
		_ErrorStatus = other._ErrorStatus;
		_filePath = other._filePath;
		if (other._cgiHandler)
			_cgiHandler = new CgiHandler(*other._cgiHandler);
		else
			_cgiHandler = NULL;
	}
	return *this;
}

HttpResponse::~HttpResponse() {
	if (_cgiHandler) {
		delete _cgiHandler;
		_cgiHandler = NULL;
	}
	if (_readFd >= 0) {
		close(_readFd);
		_readFd = -1;
	}
}


/*-------------------------
 Constructing the Response
---------------------------*/

void	HttpResponse::construct()
{	
	if (!_body.empty())
	{	
		if (_status >= 400)
		{
			_ErrorStatus.setServer(_server);
			buildErrorPage();
		}
		_response += "Content-Length: " + to_string(_body.size()) + "\r\n";
	}
	else
	{
		setFileHeaders();
		if (!_isCgiComplete)
			return;
		if (!_body.empty()) // If it's a directory listing
			_response += "Content-Length: " + to_string(_body.size()) + "\r\n";
	}
	
	// ----- Header Server -----
	_response += "Server: " + VERSION + "\r\n";

	// ----- Header Date -----
	char		date_buffer[100];
	std::time_t	now = std::time(0);
	std::tm*	gtm = std::gmtime(&now);
	std::strftime(date_buffer, sizeof(date_buffer), "%a, %d %b %Y %H:%M:%S GMT", gtm);
	_response += "Date: " + std::string(date_buffer) + "\r\n";

	// ----- Header Connection -----
	if (_connectionKeepAlive)
		_response += "Connection: keep-alive\r\n";
	else
		_response += "Connection: close\r\n";

	// ----- Custom headers -----
	std::map<std::string, std::string>::const_iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it)
		_response += it->first + ": " + it->second + "\r\n";
	
	_response += "\r\n";
	// ----- Status Line -----
	_response.insert(0, _protocol + " " + to_string(_status) + " " + _ErrorStatus.getMessage(_status) + "\r\n");
}


/*-------------------
		Headers
---------------------*/

static std::string	getMimeType(const std::string &file_path) {
	size_t		dot = file_path.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string ext = file_path.substr(dot + 1);

	if (ext == "html" || ext == "htm")
		return "text/html";
	if (ext == "css")
		return "text/css";
	if (ext == "js" || ext == "mjs")
		return "application/javascript";
	if (ext == "json")
		return "application/json";
	if (ext == "txt")
		return "text/plain";
	if (ext == "png")
		return "image/png";
	if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	if (ext == "webp")
		return "image/webp";
	if (ext == "svg")
		return "image/svg+xml";
	if (ext == "ico")
		return "image/x-icon";
	if (ext == "gif")
		return "image/gif";

	if (ext == "woff" || ext == "woff2")
		return "font/woff";
	if (ext == "ttf")
		return "font/ttf";
	if (ext == "otf")
		return "font/otf";

	if (ext == "mp4")
		return "video/mp4";
	if (ext == "webm")
		return "video/webm";
	if (ext == "mp3")
		return "audio/mpeg";
	if (ext == "ogg")
		return "audio/ogg";
	if (ext == "wav")
		return "audio/wav";
	if (ext == "flac")
		return "audio/flac";

	if (ext == "xml")
		return "application/xml";
	if (ext == "csv")
		return "text/csv";
	if (ext == "md")
		return "text/markdown";
	if (ext == "yaml" || ext == "yml")
		return "application/x-yaml";

	if (ext == "zip")
		return "application/zip";
	if (ext == "tar")
		return "application/x-tar";
	if (ext == "gz")
		return "application/gzip";
	if (ext == "bz2")
		return "application/x-bzip2";

	if (ext == "sh")
		return "application/x-sh";
	if (ext == "py")
		return "text/x-python";
	if (ext == "php")
		return "application/x-httpd-php";
	if (ext == "c")
		return "text/x-c";
	if (ext == "cpp"  || ext == "hpp" || ext == "tpp")
		return "text/x-c++src";
	if (ext == "h" || ext == "hpp")
		return "text/x-c";

	return "application/octet-stream";
}


/**
 * @brief Get file information and determine the appropriate HTTP status code.
 *
 * @param filePath The path to the file.
 * @param fileStat A struct stat to fill with file metadata.
 * @return int HTTP status code: 200 if OK, 403 if forbidden, 404 if not found, 500 if internal error.
 */
static int	getFileInfos(const std::string &filePath, struct stat &fileStat, int &fd) {

	if (stat(filePath.c_str(), &fileStat) != 0) {
		if (errno == ENOENT)
			return 404;
		return 500;
	}

	if (access(filePath.c_str(), R_OK) != 0) {
		if (errno == EACCES)
			return 403;
		return 500;
	}

	fd = open(filePath.c_str(), O_RDONLY);
	if (fd < 0) {
		if (errno == EACCES)
			return 403;
		if (errno == ENOENT)
			return 404;
		return 500;
	}

	return 200;
}

void	HttpResponse::setFileHeaders() {
	_ErrorStatus.setServer(_server);
	if (_filePath.empty())
		getFile();
	if (!_isCgiComplete)
	{
		_response.clear();
		return;
	}
	if (_status >= 400) {
		if (_ErrorStatus.getCode() == 200)
			_ErrorStatus.setCode(_status);
		_filePath = _ErrorStatus.getFilePath();
		if (_filePath.empty() || access(_filePath.c_str(), R_OK) != 0) {
			buildErrorPage();
			return;
		}
	}

	if (_filePath.empty())
		return;
	struct stat	fileStat;
	int			fd = -1;

	int fileStatus = getFileInfos(_filePath, fileStat, fd);
	if (fileStatus != 200) {
		setStatus(fileStatus);
		_filePath = _ErrorStatus.getFilePath();
		if (_status == 500)
			g_logger.log(LOG_ERROR, "Internal server error while accessing file: " + _filePath);
		if (_filePath.empty() || access(_filePath.c_str(), R_OK) != 0) {
			buildErrorPage();
			return;
		}
		return;
	}

	_readFd = fd;
	_headers["Content-Length"] = to_string(fileStat.st_size);
	_headers["Content-Type"] = getMimeType(_filePath);
}

/*-------------------
		Get File
---------------------*/

std::string HttpResponse::isCgiFile(const t_location *loc, const std::string &filepath)
{
	struct stat st;

	// Check file exists and is regular
	if (stat(filepath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
	{
		_status = 404; // File not found
		return "";
	}

	// Iterate over CGI extensions configured in location
	for (std::map<std::string, std::string>::const_iterator it = loc->_cgi.begin(); it != loc->_cgi.end(); ++it)
	{
		const std::string &ext = it->first;
		const std::string cgiBin = it->second;

		if (filepath.size() >= ext.size() &&
			filepath.compare(filepath.size() - ext.size(), ext.size(), ext) == 0)
		{
			if (access(filepath.c_str(), X_OK) == 0)
				return cgiBin;
			else
			{
				_status = 403; // Forbidden: file exists but is not executable
				return "";
			}
		}
	}
	return "";
}

void HttpResponse::getFile()
{
	// 1) Find the best matching location for the URI
	t_location *loc = findBestLocation(&_server, _uri);
	if (!loc)
	{
		setStatus(404);
		return;
	}
	if (!checkMethod(_method, loc->_methods))
	{
		setStatus(405);
		return;
	}
	if (!loc->_return_uri.empty())
	{
		// If a return URI is set, redirect to it
		setStatus(301);
		_headers["Location"] = loc->_return_uri;
		_body = "Redirecting to " + loc->_return_uri + "...";
		if (loc->_return_code != 0)
			setStatus(loc->_return_code);
		return;
	} else if (loc->_return_code != 0) {
		setStatus(loc->_return_code);
		return;
	}

	// 2) Build the URI relative to the location path
	std::string prefix = loc->_path;
	if (prefix.empty() || prefix[0] != '/') // assure prefix starts with a slash
		prefix = "/" + prefix;
	if (prefix[prefix.size()-1] != '/') // assure prefix ends with a slash
		prefix += "/";

	std::string uri = _uri;
	std::string rel = uri.substr(prefix.size()); // strip the location path from the URI

	// 3) Build the full file path
	std::string full = loc->_root;
	if (full.empty() || full[full.size()-1] != '/')
		full += "/";
	full += rel;

	struct stat st;
	bool exists = (stat(full.c_str(), &st) == 0);

	// 5) if the URI ends with a slash, we assume it's a directory
	if (!uri.empty() && uri[uri.size()-1] == '/')
	{
		// 5.a) if the directory exists, check for index files
		for (size_t i = 0; i < loc->_indexes.size(); ++i)
		{
			std::string idx = full;
			if (full[full.size()-1] != '/')
				idx += "/";
			idx += loc->_indexes[i];
			if (stat(idx.c_str(), &st) == 0 && S_ISREG(st.st_mode))
			{
				_filePath = idx;
				setStatus(200);
				return;
			}
		}
		// 5.b) autoindex ?
		if (exists && S_ISDIR(st.st_mode))
		{
			if (access(full.c_str(), R_OK) != 0)
			{
				setStatus(403); // Directory exists but cannot be read
				return;
			}
			if (loc->_autoindex)
			{
				_body = generateAutoindexPage(uri, full);
				setStatus(200);
				return;
			}
		}
		setStatus(exists ? 403 : 404);
		return;
	}
	
	// 6) if the URI does not end with slash, we assume it's a file
	if (!exists || !S_ISREG(st.st_mode))
	{
		setStatus(404);
		return;
	}

	if (_method == "DELETE")
	{
		g_logger.log(LOG_INFO, "DELETE request for file: " + full);
		setStatus(deleteFile(full));
		return;
	}

	// Check if the file should be executed as CGI
	std::string cgiBin = isCgiFile(loc, full);
	if (!cgiBin.empty())
	{
		// File is a CGI script: create the CGI handler
		std::map<std::string, std::string> envMap;
		generateEnvMap(full, envMap);
		_cgiHandler = new CgiHandler(cgiBin, full, _body, envMap, loc->_cgiTimeout);
		_cgiHandler->execute();
		_isCgiComplete = false;
		setStatus(200);
		return;
	}

	// Otherwise serve the file normally
	_filePath = full;
	setStatus(200);

}

/*-------------------------
	Buffer Sending
---------------------------*/

void HttpResponse::extractCgiHeaders(const std::string &body) {
	_headers.clear();
	_headers["Content-Type"] = "text/plain"; // Default content type if none provided

	// First check if this looks like header data (contains colon)
	bool has_headers = false;
	size_t first_line_end = body.find('\n');
	if (first_line_end != std::string::npos) {
		std::string first_line = body.substr(0, first_line_end);
		if (first_line.find(':') != std::string::npos) {
			has_headers = true;
		}
	}

	if (!has_headers) { // If no headers detected, treat entire body as content
		_body = body;
		return;
	}

	size_t headerEnd = body.find("\r\n\r\n"); // Find header-body separator
	if (headerEnd == std::string::npos) {
		headerEnd = body.find("\n\n");
		if (headerEnd == std::string::npos) {
			_body = body;
			return;
		}
	}

	// Extract headers part
	std::string headersPart = body.substr(0, headerEnd);
	std::istringstream headerStream(headersPart);
	std::string line;

	while (std::getline(headerStream, line)) {
		// Remove trailing \r if present
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos) continue;

		std::string name = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);

		// Trim leading whitespace
		value.erase(0, value.find_first_not_of(" \t"));

		// Special case: Status header
		if (name == "Status") {
			size_t codeEnd = value.find(' ');
			std::string codeStr = (codeEnd == std::string::npos) ? value : value.substr(0, codeEnd);
			setStatus(atoi(codeStr.c_str()));
		} else if (name == "Content-Length" || name == "Server" || name == "Date" || name == "Connection")
			continue; // Skip these headers as they are handled separately
		else {
			_headers[name] = value;
		}
	}

	// Extract body (skip separator)
	if (headerEnd == body.find("\r\n\r\n")) {
		_body = body.substr(headerEnd + 4); // Skip \r\n\r\n
	} else {
		_body = body.substr(headerEnd + 2); // Skip \n\n
	}
}

void	HttpResponse::handleCgi()
{
	std::string	output;
	
	if (!_cgiHandler)
		return;

	int timeout;
	_cgiHandler->readFromCgi();
	timeout = _cgiHandler->checkTimeout();
	if (_cgiHandler->isFinished())
	{
		_isCgiComplete = true;
		output = _cgiHandler->getOutput();
		setStatus(_cgiHandler->getStatusCode());
		if (!output.empty())
			extractCgiHeaders(output);
		else
		{
			if (timeout)
				setStatus(504); // Gateway Timeout
			else if (_cgiHandler->getStatusCode() == 0)
				setStatus(500); // Internal Server Error
			_body = _ErrorStatus.getMessage(_status); // It's overwrite by buildErrorPage() but necessary to not infinately loop in construct()
		}
	}
}


t_buffer	HttpResponse::sendResponse()
{
	t_buffer	buf = { NULL, 4242 };

	_isComplete = false;
	if (!_isHeadersSent)
	{
		
		if (!_isCgiComplete)
			handleCgi();
		if (!_isCgiComplete) // If the CGI has not finished yet, we cannot send the response
			return buf;
		if (_response.empty())
			construct();
		if (!_isCgiComplete) // First call Cgi object has been created
			return buf;
		if (_response.empty())
		{
			g_logger.log(LOG_ERROR, "Failed to construct response: empty response");
			return buf;
		}
		_isHeadersSent = true;

		size_t headerLen = _response.size();
		size_t maxBodySize = _server._maxBodySize - headerLen;
		size_t bodyLen = 0;
	
		if (!_body.empty())
			bodyLen = std::min(_body.size(), maxBodySize);
		else if (_readFd >= 0)
			bodyLen = maxBodySize;

		char *response = new char[bodyLen + headerLen];
		std::memcpy(response, _response.c_str(), headerLen);

		if (!_body.empty()) // If we have a body we fill the response buffer as much as possible
		{
			std::memcpy(response + headerLen, _body.c_str(), bodyLen);
			_body.erase(0, bodyLen);
			if (_body.empty() && _readFd < 0)
				_isComplete = true;
			buf.data = response;
			buf.size = bodyLen + headerLen;
			return buf;
		}
		else if (_readFd >= 0)
		{
			ssize_t n = read(_readFd, response + headerLen, bodyLen);
			if (n < 0)
			{
				g_logger.log(LOG_ERROR, "Failed to read file: " + std::string(strerror(errno)));
				delete[] response;
				return buf;
			}
			else if (n == 0 || (size_t)n < maxBodySize)
			{
				close(_readFd);
				_readFd = -1;
				_isComplete = true;
			}
			bodyLen = n;
		}
		else // If we have no body and no file to read, we consider the response complete
			_isComplete = true;

		buf.data = response;
		buf.size = headerLen + bodyLen;
		return buf;
	}
	// If we are here, it means we have already sent the headers
	return getBody();
}

t_buffer	HttpResponse::getBody()
{
	t_buffer	buf = { NULL, 0 };

	if (_readFd >= 0)
	{
		char *data = new char[_server._maxBodySize];
		ssize_t n = read(_readFd, data, _server._maxBodySize);
		if (n < 0)
		{
			g_logger.log(LOG_ERROR, "Failed to read file: " + std::string(strerror(errno)));
			delete[] data;
			return buf;
		}
		if (n == 0)
		{
			close(_readFd);
			_readFd = -1;
			_isComplete = true;
			delete[] data;
			return buf;
		}
		if ((size_t)n < _server._maxBodySize)
		{
			close(_readFd);
			_readFd = -1;
			_isComplete = true;
		}
		buf.data = data;
		buf.size = n;
	}
	else if (!_body.empty())
	{
		size_t len = std::min(_body.size(), static_cast<size_t>(_server._maxBodySize));
		char *data = new char[len];
		std::memcpy(data, _body.c_str(), len);
		_body.erase(0, len);
		if (_body.empty())
			_isComplete = true;
		buf.data = data;
		buf.size = len;
	}
	else
		_isComplete = true;

	return buf;
}


/*-------------------------
	Getters and Setters
---------------------------*/

HttpError HttpResponse::getStatus() const {
	return _status;
}

bool HttpResponse::isComplete() const {
	return _isComplete;
}

void HttpResponse::setStatus(const HttpError &status) {
	_status = status.getCode();
	_ErrorStatus = status;
}

void HttpResponse::setStatus(int code) {
	_ErrorStatus.setCode(code);
	_status = code;
}

void HttpResponse::addHeader(const std::string &name, const std::string &value) {
	_headers[name] = value;
}

void HttpResponse::setBody(const std::string &body) {
	_body = body;
}

void HttpResponse::setFilePath(const std::string &filePath) {
	_filePath = filePath;
}
