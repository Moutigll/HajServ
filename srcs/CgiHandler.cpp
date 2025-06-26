/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 21:14:48 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/26 21:29:07 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CgiHandler.hpp"

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
		return (500);

	if (_pid == 0) // Child process
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
		char **envp = _buildEnvp();
		if (!envp)
			exit(1);

		execve(_cgiPath.c_str(), argv, envp);
		_freeEnvp(envp);
		exit(1);
	}

	close(pipeIn[0]);
	close(pipeOut[1]);

	if (!_requestBody.empty()) // Write request body to CGI
		write(pipeIn[1], _requestBody.c_str(), _requestBody.size());
	close(pipeIn[1]);

	_pipeFd = pipeOut[0];
	int flags = fcntl(_pipeFd, F_GETFL, 0);
	fcntl(_pipeFd, F_SETFL, flags | O_NONBLOCK); // Set pipe to non-blocking mode to avoid blocking on read if no data is available

	_startTime = time(NULL);
	_timeout = false;
	_finished = false;

	return 0;
}

void	CgiHandler::readFromCgi()
{
	if (_pipeFd == -1 || _finished)
		return;

	char buffer[4096];
	ssize_t bytes = read(_pipeFd, buffer, sizeof(buffer));
	while (bytes > 0)
	{
		_output.append(buffer, bytes); // Empty data in the pipe
		bytes = read(_pipeFd, buffer, sizeof(buffer));
	}
	if (bytes == 0) // The process has finished writing
	{
		close(_pipeFd);
		_pipeFd = -1;

		int status;
		pid_t ret;

		ret = waitpid(_pid, &status, WNOHANG);
		if (ret == 0)
		{
			// Process still running
			// We can return here, the process is still running and we will check again later
			return;
		}
		else if (ret == _pid) // Processus finished
		{
			_finished = true;
			_pid = -1;

			if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
				_statusCode = 200;
			else
				_statusCode = 500;
		}
		else // Error in waitpid
		{
			_finished = true;
			_pid = -1;
			_statusCode = 500;
		}
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

		ret = waitpid(_pid, NULL, WNOHANG);

		if (ret == _pid || ret == -1)
		{
			_pid = -1;

			if (_pipeFd != -1)
				close(_pipeFd);
			_pipeFd = -1;

			_timeout = true;
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

char	**CgiHandler::_buildEnvp()
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
			_freeEnvp(envp);
			return NULL;
		}
		i++;
	}
	envp[i] = NULL;
	return envp;
}

void	CgiHandler::_freeEnvp(char **envp)
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
