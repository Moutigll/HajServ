/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HandleCgi.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 01:41:46 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/26 04:01:54 by etaquet          ###   ########.fr       */
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
	(void)timeout;

    if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) {
        g_logger.log(LOG_ERROR, "pipe() failed: " + std::string(strerror(errno)));
        return false;
    }

    pid_t pid = fork();
	_connection->setCgiPid(pid);
	
    if (pid < 0) {
        g_logger.log(LOG_ERROR, "fork() failed: " + std::string(strerror(errno)));
        close(pipeIn[0]); close(pipeIn[1]);
        close(pipeOut[0]); close(pipeOut[1]);
        return false;
    }

    if (pid == 0) {
        // Child process
        close(pipeIn[1]);
        close(pipeOut[0]);

        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);
        close(pipeIn[0]);
        close(pipeOut[1]);

        char *argv[3] = {
            const_cast<char*>(Command.c_str()),
            const_cast<char*>(scriptPath.c_str()),
            NULL
        };

        std::vector<std::string> envVars;
        char **envp = buildCGIEnv(envVars, scriptPath);

        execve(argv[0], argv, envp);
        perror("execve");
        _exit(1);
    }

    // Parent process
    close(pipeIn[0]);
    close(pipeOut[1]);

    // Write POST data if applicable
    // if (_method == "POST" && !_body.empty()) {
    //     write(pipeIn[1], _body.c_str(), _body.size());
    // }
    close(pipeIn[1]);

    // Configure non-blocking read
    int flags = fcntl(pipeOut[0], F_GETFL, 0);
    if (fcntl(pipeOut[0], F_SETFL, flags | O_NONBLOCK) == -1) {
        g_logger.log(LOG_ERROR, "fcntl() failed: " + std::string(strerror(errno)));
        close(pipeOut[0]);
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return false;
    }

    // Store CGI state in connection
    _connection->setCgiState(pid, pipeOut[0], timeout);
    _body.clear();
    
    return true;
}