/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HandleCgi.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 01:41:46 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/26 01:42:30 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Http/HttpResponse.hpp"

static volatile sig_atomic_t timeout_occurred = 0;

extern "C" void alarm_handler(int) {
	timeout_occurred = 1;
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