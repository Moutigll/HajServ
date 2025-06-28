/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 21:14:48 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/28 08:06:18 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CgiHandler.hpp"
#include "../includes/Logger.hpp"

CgiHandler::CgiHandler()
	: _timeout(5),
	_pid(-1),
	_pipeFd(-1),
	_startTime(0),
	_finished(false) {}

CgiHandler::CgiHandler(const std::string &cgiPath,
					   const std::string &scriptPath,
					   const std::string &requestBody,
					   const std::map<std::string, std::string> &envMap,
					   int timeout)
	: _cgiPath(cgiPath),
	  _scriptPath(scriptPath),
	  _requestBody(requestBody),
	  _envMap(envMap),
	  _timeout(timeout),
	  _pid(-1),
	  _pipeFd(-1),
	  _startTime(0),
	  _finished(false) {}

CgiHandler::CgiHandler(const CgiHandler &other)
{
	*this = other;
}

CgiHandler &CgiHandler::operator=(const CgiHandler &other)
{
	if (this != &other)
	{
		_cgiPath = other._cgiPath;
		_scriptPath = other._scriptPath;
		_requestBody = other._requestBody;
		_envMap = other._envMap;
		_output = other._output;
		_timeout = other._timeout;
		_pid = other._pid;
		_pipeFd = other._pipeFd;
		_startTime = other._startTime;
		_finished = other._finished;
	}
	return *this;
}

CgiHandler::~CgiHandler()
{
	if (_pipeFd != -1)
		close(_pipeFd);
	if (_pid != -1)
		kill(_pid, SIGKILL);
}

int	CgiHandler::execute()
{
	int pipeIn[2]; // Send data to CGI
	int pipeOut[2]; // Receive data from CGI
	if (pipe(pipeOut) == -1 || pipe(pipeIn) == -1)
		return (500);

	_pid = fork();
	if (_pid < 0)
	{
		close(pipeIn[0]); close(pipeIn[1]);
		close(pipeOut[0]); close(pipeOut[1]);
		return (500);
	}
	if (!_pid) // Child process
	{
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeIn[1]);
		close(pipeOut[0]);
		
		char *argv[] = {
			const_cast<char *>(_cgiPath.c_str()),
			const_cast<char *>(_scriptPath.c_str()),
			NULL
		};
		char **envp = buildEnvp();
		if (!envp)
			exit(1);
		execve(_cgiPath.c_str(), argv, envp);
		g_logger.log(LOG_ERROR, "Error executing CGI script: " + _cgiPath + " - " + std::string(strerror(errno)));
		freeEnvp(envp);
		exit(1);
	}

	close(pipeIn[0]);
	close(pipeOut[1]);

	if (!_requestBody.empty()) // Write request body to CGI
	{
	 	ssize_t bytesWritten =  write(pipeIn[1], _requestBody.c_str(), _requestBody.size());
		if (bytesWritten < 0)
			g_logger.log(LOG_ERROR, "Error writing to CGI pipe: " + std::string(strerror(errno)));
	}
	close(pipeIn[1]);

	_pipeFd = pipeOut[0];
	// Set pipe to non-blocking mode to avoid blocking on read if no data is available
	fcntl(_pipeFd, F_SETFL, O_NONBLOCK);

	_startTime = time(NULL);
	_finished = false;

	return 0;
}

void CgiHandler::handleReadError(int bytes)
{
	if ((bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) || bytes > 0)
		return; // No data available, return and check again later
	if (bytes == 0) // No more data to read, process has finished
	{
		close(_pipeFd);
		_pipeFd = -1;
		if (_pid > 0)
		{
			int status;
			pid_t result = waitpid(_pid, &status, WNOHANG); // Non-blocking
			if (result == -1)
			{
				g_logger.log(LOG_ERROR, "Error waiting for CGI process: " + std::string(strerror(errno)));
				_statusCode = 500;
				_finished = true;
			}
			else if (result > 0)
			{
				if (WIFEXITED(status))
					_statusCode = WEXITSTATUS(status);
				else if (WIFSIGNALED(status))
					_statusCode = 500; // Internal Server Error
				_finished = true;
			}
			else
				return; // Process still running, wait for next read
		}
	}
	else if (bytes < 0) // Error in read
	{
		g_logger.log(LOG_ERROR, "Error reading from CGI pipe: " + std::string(strerror(errno)));
		_finished = true;
		_pid = -1;
		_statusCode = 500;
	}
}

void	CgiHandler::readFromCgi()
{
	size_t bytesRead = 0;
	int status;
	pid_t result = 0;
	if (_pid > 0)
	{
		result = waitpid(_pid, &status, WNOHANG);
		if (result == _pid) // If the CGI process has finished waitpid returns the same PID
		{
			_finished = true;
			_pid = -1;
			_statusCode = (status == 0 ? 200 : 500); // If the process exited normally, set status code to 200, otherwise 500
		}
	}
	if (_pipeFd == -1 || _finished)
		return;
	char buffer[4096];
	ssize_t bytes = read(_pipeFd, buffer, sizeof(buffer));
	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return; // No data available, return and check again later
		else
		{
			g_logger.log(LOG_ERROR, "Error reading from CGI pipe: " + std::string(strerror(errno)));
			//_finished = true;
			_pid = -1;
			_statusCode = 500;
			return;
		}
	}
	while (bytes > 0)
	{
		bytesRead += bytes;
		if (bytesRead > 65535) // Prevent buffer overflow, max 16KB output
		{
			g_logger.log(LOG_ERROR, "CGI output too large, aborting.");
			_finished = true;
			_pid = -1;
			_statusCode = 504; // Gateway Timeout
			_output.append(buffer, bytes);
			break;
		}
		_output.append(buffer, bytes); // Empty data in the pipe
		bytes = read(_pipeFd, buffer, sizeof(buffer));
	}
	
}

bool	CgiHandler::checkTimeout(void)
{
	int	ret;
	bool	hasTimedOut;

	if (_finished || _pid == -1)
		return (false);
	if (time(NULL) - _startTime >= _timeout)
	{
		kill(_pid, SIGKILL);

		ret = waitpid(_pid, NULL, 0);

		if (ret == _pid || ret == -1)
		{
			_pid = -1;

			if (_pipeFd != -1)
				close(_pipeFd);
			_pipeFd = -1;
			_statusCode = 504;
			_finished = true;

			hasTimedOut = true;
		}
		else
			hasTimedOut = false;
		return (hasTimedOut);
	}
	return (false);
}

char	**CgiHandler::buildEnvp()
{
	char **envp = (char **)malloc(sizeof(char *) * (_envMap.size() + 1));
	if (!envp)
		return NULL;

	size_t i = 0;
	for (std::map<std::string, std::string>::const_iterator it = _envMap.begin(); it != _envMap.end(); ++it)
	{
		std::string str = it->first + "=" + it->second;
		envp[i] = strdup(str.c_str());
		if (!envp[i])
		{
			freeEnvp(envp);
			return NULL;
		}
		i++;
	}
	envp[i] = NULL;
	return envp;
}

void	CgiHandler::freeEnvp(char **envp)
{
	if (!envp)
		return;
	for (size_t i = 0; envp[i]; ++i)
		free(envp[i]);
	free(envp);
}


/*-------------------------
	Getters and Setters
---------------------------*/

bool	CgiHandler::isFinished() const {
	return _finished;
}

const std::string&	CgiHandler::getOutput() const {
	return _output;
}

bool	CgiHandler::hasTimedOut() const {
	return _timeout;
}

int	CgiHandler::getStatusCode() {
	if (_finished)
		return _statusCode;
	return 0;
}

