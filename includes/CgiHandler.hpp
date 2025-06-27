/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 20:15:41 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/26 21:28:26 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

# include <string>
# include <map>
# include <vector>
# include <unistd.h>
# include <fcntl.h>
# include <cstring>
# include <cstdlib>
# include <sys/wait.h>
# include <signal.h>
# include <ctime>
# include <iostream>
#include "Logger.hpp"

/**
 * @class CgiHandler
 * @brief Manages the execution of CGI scripts with timeout and output capture capabilities
 */
class CgiHandler {
	public:
		CgiHandler();
		CgiHandler(const std::string &cgiPath,
				   const std::string &scriptPath,
				   const std::string &requestBody,
				   const std::map<std::string, std::string> &envMap,
				   int timeout);
		CgiHandler(const CgiHandler &other);
		CgiHandler &operator=(const CgiHandler &other);
		~CgiHandler();

		/**
		 * @brief Executes the CGI script in a child process
		 * 
		 * This method forks a child process to execute the CGI script, sets up pipes for communication,
		 * and configures the environment. The parent process keeps track of the execution.
		 * 
		 * @return int Returns 0 on success, 500 on fork/pipe creation failure
		 * @note The method sets up non-blocking I/O on the output pipe
		 */
		int								execute();
		
		/**
		* @brief Reads available output from the CGI script
		* 
		* Continuously reads from the CGI output pipe while data is available. When the pipe is closed,
		* it checks the process status and updates the completion state. Handles both successful
		* and failed process termination.
		* 
		* @note This method should be called repeatedly until isFinished() returns true
		*/
		void							readFromCgi();

		/**
		 * @brief Checks if the CGI script has exceeded its execution timeout
		 * 
		 * Compares the current time against the start time plus timeout duration. If exceeded,
		 * kills the CGI process and cleans up resources.
		 * 
		 * @return bool True if the process was terminated due to timeout, false otherwise
		 * @warning This method will SIGKILL the process if timeout occurs
		 */
		bool							checkTimeout();
		bool							hasTimedOut() const;
		const std::string&				getOutput() const;
		bool							isFinished() const;
		int								getStatusCode();


	private:
		std::string							_cgiPath; // Path to the CGI binary
		std::string							_scriptPath; // Path to the script to be executed by CGI
		std::string							_requestBody; // Body of the HTTP request to be sent to the CGI script
		std::map<std::string, std::string>	_envMap; // Environment variables to be set for the CGI script
		std::string							_output; // Output from the CGI script

		int		_statusCode;
		int		_timeout; // Timeout in seconds for the CGI execution
		pid_t	_pid; // Process ID of the CGI script
		int		_pipeFd; // File descriptor for the pipe to read output from the CGI script
		time_t	_startTime; // Start time of the CGI execution
		bool	_finished; // Flag to indicate if the CGI execution is finished


		/**
		 * @brief Builds the environment variable array for the CGI process
		 * 
		 * Converts the internal environment map to a NULL-terminated char** array suitable
		 * for execve(). Each entry is formatted as "KEY=VALUE".
		 * 
		 * @return char** Allocated environment array, or NULL on allocation failure
		 * @warning The caller is responsible for freeing the returned array with _freeEnvp()
		 * @see _freeEnvp
		 */
		char							**_buildEnvp();

		void							_freeEnvp(char **envp);
};

#endif
