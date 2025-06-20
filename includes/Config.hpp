/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 04:01:24 by etaquet           #+#    #+#             */
/*   Updated: 2025/06/20 16:39:20 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Logger.hpp"

typedef struct s_location
{
	std::string							_path;
	std::map<std::string, std::string>	_loc_data;
	int									_return_code;
	std::string							_return_uri;
	bool								_autoindex;
	std::vector<std::string>			_methods;
	std::vector<std::string>			_try_files;
}	t_location;

typedef struct s_server
{
	std::vector<std::string>			_methods;
	std::vector<std::string>			_hosts;
	std::vector<std::string>			_indexes;
	std::map<std::string, std::string>	_data;
	std::map<int, std::string>			_errors;
	std::string							_root_error;
	std::vector<t_location>				_locations;
	std::vector<int>					_ports;
	size_t								_timeout;
	size_t								_max_body_size;
}	t_server;

class Config
{
	public:
		Config();
		Config( Config const & );
		Config & operator=(Config const & );
		~Config();

		/**
		 * @brief Parses the configuration file.
		 *
		 * Opens the file, reads it line by line, and parses global
		 * directives and server blocks.
		 *
		 * @param filename The path to the config file.
		 * @return true if parsing was successful, false otherwise.
		 */
		bool						parse(const std::string &filename);
		const t_server				&getServerBlock(size_t index) const;
		const std::vector<t_server>	&getServers() const;
		void						logConfig() const;

		size_t	getServerCount() const;
	private:
		/**
		 * @brief Parses global directives outside of server blocks.
		 *
		 * Currently supports logging directives such as log_connections,
		 * log_request, log_console, log_file, and log_level.
		 *
		 * @param line The config line to parse.
		 * @param line_number The current line number for error messages.
		 * @return true if the global directive was parsed successfully, false otherwise.
		 */
		bool	parseGlobals(std::string &line, int &line_number);

		/**
		 * @brief Parses a server block from the config file.
		 *
		 * Reads lines until the server block end '};' is found.
		 * Parses server-level directives and nested location blocks.
		 *
		 * @param file The opened config file stream.
		 * @param line_number Reference to the current line number (updated as parsing proceeds).
		 * @return true if the server block was parsed successfully, false on error.
		 */
		bool	parseServer(std::ifstream &file, int &line_number);

		/**
		 * @brief Parses a location block inside a server.
		 *
		 * Reads lines until the location block end '}' is found.
		 * Parses directives related to this location.
		 *
		 * @param file The opened config file stream.
		 * @param line_number Reference to the current line number.
		 * @param loc The location struct to fill with parsed data.
		 * @return true if the location block was parsed successfully, false otherwise.
		 */
		bool	parseLocation(std::ifstream &file, int &line_number, t_location &loc);

		/**
		 * @brief Parses error pages block inside a server.
		 *
		 * Reads lines until '}' is found. Maps HTTP error codes to error page paths.
		 *
		 * @param file The opened config file stream.
		 * @param line_number Reference to the current line number.
		 * @param server The server struct to update with error pages.
		 * @return true if error pages block parsed successfully, false otherwise.
		 */
		bool	parseErrors(std::ifstream &file, int &line_number, t_server &server);
		bool	parseVectors(std::string &line, t_server &server, t_location &loc, bool is_loc);

		bool					_finished;
		bool					_log_connections;
		bool					_log_request;
		bool					_log_console;
		std::string				_log_file;
		std::string				_log_level;
		std::vector<t_server>	_servers;
};

#endif
