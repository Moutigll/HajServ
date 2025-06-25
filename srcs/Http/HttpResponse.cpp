/* ************************************************************************** */
/*																			*/
/*														:::	  ::::::::   */
/*   HttpResponse.cpp								   :+:	  :+:	:+:   */
/*													+:+ +:+		 +:+	 */
/*   By: ele-lean <ele-lean@student.42.fr>		  +#+  +:+	   +#+		*/
/*												+#+#+#+#+#+   +#+		   */
/*   Created: 2025/06/18 18:40:42 by ele-lean		  #+#	#+#			 */
/*   Updated: 2025/06/23 21:20:51 by ele-lean		 ###   ########.fr	   */
/*																			*/
/* ************************************************************************** */

#include "../../includes/Http/HttpResponse.hpp"

HttpResponse::HttpResponse(const t_server &server)
	: HttpTransaction(), _server(server), _isHeadersSent(false), _readFd(-1)
{
	_status = 200; // Default status code
	_protocol = "HTTP/1.1";
	_isComplete = false;
}

HttpResponse::HttpResponse(const t_server &server, HttpRequest &request)
	: HttpTransaction(request), _server(server), _isHeadersSent(false), _readFd(-1) {
	_method = request.getMethod();
	_request = request.getRequest();
	_protocol = request.getProtocol();
	_headers.clear();
}

HttpResponse::HttpResponse(const HttpResponse &other)
	: HttpTransaction(other),
	  _server(other._server),
	  _response(other._response),
	  _ErrorStatus(other._ErrorStatus),
	  _filePath(other._filePath),
	  _isHeadersSent(other._isHeadersSent),
	  _readFd(other._readFd) {}

HttpResponse &HttpResponse::operator=(const HttpResponse &other) {
	if (this != &other) {
		HttpTransaction::operator=(other);
		_server = other._server;
		_isHeadersSent = other._isHeadersSent;
		_readFd = other._readFd;
		_response = other._response;
		_ErrorStatus = other._ErrorStatus;
	}
	return *this;
}

HttpResponse::~HttpResponse() {}


void HttpResponse::getFile()
{
	std::string	relative_path;
	t_location	*location;

	location = findBestLocation(&_server, _request);
	if (!location)
	{
		setStatus(404);
		return;
	}
	if (!checkMethod(_method, location->_methods))
	{
		setStatus(405);
		return;
	}

	relative_path = stripLocationPrefix(_request, location->_path);
	if (!relative_path.empty() && relative_path[0] == '/')
		relative_path = relative_path.substr(1);

	if (_request.size() > 0 && _request[_request.size() - 1] == '/')
	{
		if (!location->_indexes.empty())
			_filePath = joinPaths(location->_root, joinPaths(relative_path, location->_indexes[0]));
		else if (location->_autoindex)
		{
			_body = "<html><body><h1>Directory listing not implemented</h1></body></html>";
			setStatus(200);
			return;
		}
		else
		{
			setStatus(403);
			return;
		}
	}
	else
		_filePath = joinPaths(location->_root, relative_path);


	g_logger.log(LOG_DEBUG, "Serving file: " + _filePath);
	if (_method == "GET")
	{
		if (access(_filePath.c_str(), F_OK) != 0)
		{
			setStatus(404);
			return;
		}
	}

	setStatus(200);
}

void	HttpResponse::construct()
{
	_response.clear();
	_response += _protocol + " " + to_string(_status) + " " + _ErrorStatus.getMessage(_status) + "\r\n";
	
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
}


HttpError HttpResponse::getStatus() const {
	return _status;
}

t_buffer	HttpResponse::getBody()
{
	t_buffer	buf = { NULL, 0 };

	if (_readFd >= 0)
	{
		char *data = new char[_server._max_body_size];
		ssize_t n = read(_readFd, data, _server._max_body_size);
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
		if ((size_t)n < _server._max_body_size)
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
		size_t len = std::min(_body.size(), static_cast<size_t>(_server._max_body_size));
		char *data = new char[len];
		std::memcpy(data, _body.c_str(), len);
		_body.erase(0, len);
		if (_body.empty())
			_isComplete = true;
		buf.data = data;
		buf.size = len;
	}
	else
	{
		_isComplete = true;
	}

	return buf;
}

t_buffer	HttpResponse::sendResponse()
{
	t_buffer	buf = { NULL, 0 };

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

		size_t header_len = _response.size();
		size_t max_body_size = _server._max_body_size;
		size_t total_size = header_len;

		bool has_body = !_body.empty();
		bool has_file = _readFd >= 0;

		size_t body_len = 0;
		if (has_body)
			body_len = std::min(_body.size(), max_body_size);
		else if (has_file)
			body_len = max_body_size - header_len;

		total_size += body_len;

		char *response_copy = new char[total_size];
		std::memcpy(response_copy, _response.c_str(), header_len);

		if (has_body)
		{
			std::memcpy(response_copy + header_len, _body.c_str(), body_len);
			_body.erase(0, body_len);
			if (_body.empty() && !has_file)
				_isComplete = true;
		}
		else if (has_file)
		{
			ssize_t n = read(_readFd, response_copy + header_len, body_len);
			if (n < 0)
			{
				g_logger.log(LOG_ERROR, "Failed to read file: " + std::string(strerror(errno)));
				delete[] response_copy;
				return buf;
			}
			body_len = n;
			_isComplete = false;
			if (n == 0 || (size_t)n < max_body_size - header_len)
			{
				close(_readFd);
				_readFd = -1;
				_isComplete = true;
			}
		}
		else
			_isComplete = true;

		buf.data = response_copy;
		buf.size = header_len + body_len;
		return buf;
	}
	return getBody();
}


/*-------------------
		Headers
---------------------*/
static std::string	get_mime_type(const std::string &file_path) {
	size_t		dot = file_path.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string ext = file_path.substr(dot + 1);

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

static bool	get_file_info(const std::string &file_path, struct stat &file_stat, int &fd) {
	if (stat(file_path.c_str(), &file_stat) != 0)
		return false;

	fd = open(file_path.c_str(), O_RDONLY);
	if (fd < 0)
		return false;

	return true;
}

void	HttpResponse::buildErrorPage() {
	_body = HTML_HEADER;
	_body += "<h1> Error " + to_string(_status) + "</h1>\n";
	_body += "<p>" + _ErrorStatus.getMessage(_status) + "</p>\n";
	_body += HTML_FOOTER;
}

void	HttpResponse::setFileHeaders() {
	if (_filePath.empty())
		getFile();

	if (_status >= 400) {
		if (_ErrorStatus.getCode() == 200)
			_ErrorStatus.setCode(_status);
		_ErrorStatus.setServer(_server);
		_filePath = _ErrorStatus.getFilePath();
		if (_filePath.empty()) {
			buildErrorPage();
			_headers["Content-Type"] = "text/html";
			g_logger.log(LOG_WARNING, "No error page defined for status code " + to_string(_status));
			return;
		}
	}

	if (_filePath.empty())
		return;

	struct stat	file_stat;
	int			fd = -1;

	if (!get_file_info(_filePath, file_stat, fd)) {
		if (fd >= 0)
			close(fd);
		setStatus(404);
		buildErrorPage();
		_headers["Content-Type"] = "text/html";
		return;
	}

	_headers["Content-Length"] = to_string(file_stat.st_size);
	_readFd = fd;

	_headers["Content-Type"] = get_mime_type(_filePath);
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
