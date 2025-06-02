/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ele-lean <ele-lean@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 18:29:38 by ele-lean          #+#    #+#             */
/*   Updated: 2025/06/02 19:35:54 by ele-lean         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "Colors.hpp"
#include "Utils.hpp"

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

		void init(const std::string &file_path, e_log_level level, bool to_console);
		void close(void);
		void log(e_log_level level, const std::string &msg);

	private:
		std::ofstream	m_log_file;
		e_log_level		m_level;
		bool			m_to_console;

		std::string		levelToString(e_log_level level) const;

		Logger(const Logger &src);
		Logger &operator=(const Logger &rhs);
};

extern Logger g_logger;

#endif
