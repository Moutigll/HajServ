/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:40:42 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/26 21:06:56 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/HttpResponse.hpp"
#include <ostream>

HttpResponse::HttpResponse(const t_server &server)
	: HttpTransaction(),
	  _server(server),
	  _response(),
	  _filePath(),
	  _readFd(-1),
	  _isHeadersSent(false),
	  _ErrorStatus(),
	  _cgiHandler(NULL)
{
	_status = 200;
	_protocol = "HTTP/1.1";
	_isComplete = false;
}

HttpResponse::HttpResponse(const t_server &server, HttpRequest &request)
	: HttpTransaction(request),
	  _server(server),
	  _response(),
	  _filePath(),
	  _readFd(-1),
	  _isHeadersSent(false),
	  _ErrorStatus(),
	  _cgiHandler(NULL)
	  
{
	_method = request.getMethod();
	_uri = request.getRequest();
	_protocol = request.getProtocol();
	_headers.clear(); // Clear headers from the request, we will build our own response headers
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
}



/*-------------------------
 Constructing the Response
---------------------------*/

void	HttpResponse::construct()
{
	_response.clear();
	
	if (!_body.empty())
		_response += "Content-Length: " + to_string(_body.size()) + "\r\n";
	else
	{
		setFileHeaders();
		if (!_body.empty()) // If it's a directory listing
			_response += "Content-Length: " + to_string(_body.size()) + "\r\n" \
			+ "Content-Type: text/html\r\n";
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

static std::string	getMimeType(const std::string &filePath) {
	size_t		dot = filePath.find_last_of('.');
	if (dot == std::string::npos) // No extension found
		return "application/octet-stream";

	std::string ext = filePath.substr(dot + 1);

	if (ext == "html" || ext == "htm")
		return "text/html";
	if (ext == "css")
		return "text/css";
	if (ext == "js")
		return "application/javascript";
	if (ext == "png")
		return "image/png";
	if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	if (ext == "gif")
		return "image/gif";
	if (ext == "svg")
		return "image/svg+xml";
	if (ext == "mp4")
		return "video/mp4";

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
	std::cout << "status: " << _status << std::endl;
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

void HttpResponse::getFile()
{
	// 1) Find the best matching location for the URI
	t_location *loc = findBestLocation(&_server, _uri);
	std::cout << loc->_path << std::endl;
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
	
	// 6) if the URI does not end with a slash, we assume it's a file
	if (!exists || !S_ISREG(st.st_mode))
	{
		setStatus(404);
		return;
	}

	_filePath = full;
	setStatus(200);
}

/*-------------------------
	Buffer Sending
---------------------------*/

t_buffer	HttpResponse::sendResponse()
{
	t_buffer	buf = { NULL, 0 };

	_isComplete = false;
	if (!_isHeadersSent)
	{
		if (_response.empty())
			construct();
		if (_response.empty())
		{
			g_logger.log(LOG_ERROR, "Failed to construct response: empty response");
			return buf;
		}
		_isHeadersSent = true;

		size_t headerLen = _response.size();
		size_t maxBodySize = _server._maxBodySize;
		size_t total_size = headerLen;

		bool hasBody = !_body.empty();
		bool hasFile = _readFd >= 0;

		size_t bodyLen = 0;
		if (hasBody)
			bodyLen = std::min(_body.size(), maxBodySize - headerLen);
		else if (hasFile)
			bodyLen = maxBodySize - headerLen;

		total_size += bodyLen;

		char *responseCopy = new char[total_size];
		std::memcpy(responseCopy, _response.c_str(), headerLen);

		if (hasBody)
		{
			std::memcpy(responseCopy + headerLen, _body.c_str(), bodyLen);
			_body.erase(0, bodyLen);
			if (_body.empty() && !hasFile)
				_isComplete = true;
			buf.data = responseCopy;
			buf.size = total_size;
			return buf;
		}
		else if (hasFile)
		{
			ssize_t n = read(_readFd, responseCopy + headerLen, bodyLen);
			if (n < 0)
			{
				g_logger.log(LOG_ERROR, "Failed to read file: " + std::string(strerror(errno)));
				delete[] responseCopy;
				return buf;
			}
			bodyLen = n;
			if (n == 0 || (size_t)n < maxBodySize - headerLen)
			{
				close(_readFd);
				_readFd = -1;
				_isComplete = true;
			}
		}
		else
			_isComplete = true;

		buf.data = responseCopy;
		buf.size = headerLen + bodyLen;
		return buf;
	}
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
