/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 18:29:38 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/28 10:17:58 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <string>

enum e_log_level
{
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_CRITICAL
};

class Logger {
	public:
		Logger(void);
		~Logger(void);
		
		/**
		 * @brief Initializes the logger with a log file path, log level, and console output option.
		 *
		 * Opens the specified file in append mode if `file_path` is not empty,
		 * and sets the logging level and console output flag accordingly.
		 *
		 * @param[in] file_path Path to the log file. If empty, logs will not be written to a file.
		 * @param[in] level Minimum log level to output (e.g., LOG_INFO, LOG_ERROR, etc.).
		 * @param[in] to_console Whether to also print logs to the console.
		 */
		void init(const std::string &file_path, e_log_level level, bool to_console);
		void close(void);
		/**
		 * @brief Logs a message with the given level, including a timestamp.
		 *
		 * If the log level is greater than or equal to the current threshold, the message is logged.
		 * The message is printed to the console if enabled and/or written to the log file if open.
		 *
		 * @param[in] level The severity level of the message.
		 * @param[in] msg The message to log.
		 */
		void log(e_log_level level, const std::string &msg);

	private:
		std::ofstream	m_log_file;
		e_log_level		m_level;
		bool			m_to_console;

		/**
		 * @brief Converts a log level enum value to a colored string label.
		 *
		 * Returns a human-readable, ANSI-colored label corresponding to the log level.
		 *
		 * @param[in] level The log level to convert.
		 * @return std::string Colored string representation of the log level.
		*/
		std::string		levelToString(e_log_level level) const;

		Logger(const Logger &src);
		Logger &operator=(const Logger &rhs);
};

extern Logger g_logger;

#endif
