/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/18 18:40:42 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/26 01:18:19 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/HttpResponse.hpp"
#include <ostream>

static volatile sig_atomic_t timeout_occurred = 0;

extern "C" void alarm_handler(int) {
	timeout_occurred = 1;
}

HttpResponse::HttpResponse(const t_server &server)
	: HttpTransaction(),
	  _server(server),
	  _response(),
	  _filePath(),
	  _readFd(-1),
	  _isHeadersSent(false),
	  _ErrorStatus()
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
	  _ErrorStatus()
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
	  _ErrorStatus(other._ErrorStatus)
{}

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
	t_location *cgi = GetCgi(&_server, _uri);

	if (cgi)
	{
		std::string full = cgi->_loc_data["cgi_root"] + _uri;
		int timeout = atoi(cgi->_loc_data["fastcgi_read_timeout"].c_str());
		if (!timeout)
			timeout = 60;
		if (!executeCGI(cgi->_loc_data["cgi_pass"], full, timeout))
			return;
		return;
	}
	
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

char **HttpResponse::buildCGIEnv(std::vector<std::string> &envVec,
	const std::string &scriptPath) const
{
	envVec.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envVec.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envVec.push_back("REQUEST_METHOD=" + _method);
	envVec.push_back("SCRIPT_FILENAME=" + scriptPath);
	envVec.push_back("SCRIPT_NAME=" + _uri);
	envVec.push_back("SERVER_SOFTWARE=" + VERSION);

	// Query string
	size_t pos = _uri.find('?');
	if (pos != std::string::npos)
		envVec.push_back("QUERY_STRING=" + _uri.substr(pos + 1));
	else
		envVec.push_back("QUERY_STRING=");

	// Content
	if (_method == "POST") {
		envVec.push_back("CONTENT_LENGTH=" + to_string(_body.size()));
		envVec.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
	} else {
		envVec.push_back("CONTENT_LENGTH=0");
	}

	// Additional headers (optional)
	std::map<std::string, std::string>::const_iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it) {
		std::string key = it->first;
		std::string val = it->second;

		for (size_t i = 0; i < key.length(); ++i)
			if (key[i] == '-') key[i] = '_';

		std::string header = "HTTP_" + key + "=" + val;
		envVec.push_back(header);
	}

	// Convert to char**
	char **envp = new char*[envVec.size() + 1];
	for (size_t i = 0; i < envVec.size(); ++i)
		envp[i] = strdup(envVec[i].c_str());
	envp[envVec.size()] = NULL;

	return envp;
}

bool HttpResponse::executeCGI(const std::string &Command, const std::string &scriptPath, int timeout) {
	int pipeIn[2];
	int pipeOut[2];

	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) {
		g_logger.log(LOG_ERROR, "pipe() failed: " + std::string(strerror(errno)));
		return false;
	}

	pid_t pid = fork();
	if (pid < 0) {
		g_logger.log(LOG_ERROR, "fork() failed: " + std::string(strerror(errno)));
		close(pipeIn[0]); close(pipeIn[1]);
		close(pipeOut[0]); close(pipeOut[1]);
		return false;
	}

	if (pid == 0) {
		// Child
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);

		close(pipeIn[1]);
		close(pipeOut[0]);

		char *argv[3];
		argv[0] = (char*)Command.c_str();
		argv[1] = (char*)scriptPath.c_str();
		argv[2] = NULL;

		std::vector<std::string> envVars;
		char **envp = buildCGIEnv(envVars, scriptPath);

		execve(argv[0], argv, envp);
		perror("execve");
		_exit(1);
	}

	// Parent
	close(pipeIn[0]);
	close(pipeOut[1]);

	if (_method == "POST" && !_body.empty())
		write(pipeIn[1], _body.c_str(), _body.size());
	close(pipeIn[1]);

	// Setup alarm signal handler
	struct sigaction sa;
	sa.sa_handler = alarm_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	timeout_occurred = 0;
	alarm(timeout); // Start 10 second timer

	char buffer[4096];
	ssize_t n;
	_body.clear();

	while (!timeout_occurred && (n = read(pipeOut[0], buffer, sizeof(buffer))) > 0) {
		_body.append(buffer, n);
	}

	close(pipeOut[0]);

	int status;
	while (!timeout_occurred) {
		if (waitpid(pid, &status, 0) == pid)
			break;
		if (errno != EINTR) {
			g_logger.log(LOG_ERROR, "waitpid failed: " + std::string(strerror(errno)));
			break;
		}
	}

	alarm(0); // Cancel the alarm

	if (timeout_occurred) {
		_body.clear();
		kill(pid, SIGKILL);
		setStatus(504);
		g_logger.log(LOG_ERROR, "CGI script timed out");
		return false;
	}

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		setStatus(500);
		g_logger.log(LOG_ERROR, "CGI script exited abnormally");
		return false;
	}

	return true;
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

		size_t header_len = _response.size();
		size_t max_body_size = _server._max_body_size;
		size_t total_size = header_len;

		bool has_body = !_body.empty();
		bool has_file = _readFd >= 0;

		size_t body_len = 0;
		if (has_body)
			body_len = std::min(_body.size(), max_body_size - header_len);
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
			buf.data = response_copy;
			buf.size = total_size;
			return buf;
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
