/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: etaquet <etaquet@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 04:01:24 by etaquet           #+#    #+#             */
/*   Updated: 2025/05/23 04:34:47 by etaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Utils.hpp"

typedef struct s_location {
	std::string path;
	std::string root;
	std::vector<std::string> index;
	std::string alias;
	std::vector<std::string> try_files;
	int return_code;
	std::string return_url;
	bool autoindex;
	std::vector<std::string> allowed_methods;
	std::string cgi_pass;

	s_location() : return_code(0), autoindex(false) {}
}	t_location;

typedef struct s_server {
	std::vector<std::string> server_name;
	std::string listen;
	int timeout;
	std::vector<std::string> allowed_methods;
	std::map<int, std::string> error_pages;
	std::string root;
	std::vector<std::string> index;
	long client_max_body_size;
	std::string access_log;
	std::string error_log;
	std::vector<t_location> locations;

	s_server() : timeout(0), client_max_body_size(0) {}
}	t_server;

class Config {
	public:
		bool parse(const std::string &filename);
		const std::vector<t_server>& getServers() const;
		const t_server& getServer(size_t index) const;
		size_t getServerCount() const;

	private:
		std::vector<t_server> servers;
		struct Token {
			std::string value;
			int line;
		};
		std::vector<Token> tokenize(std::ifstream &file);
		bool parseServerBlock(std::vector<Token> &tokens, size_t &i, t_server &server);
		bool parseLocationBlock(std::vector<Token> &tokens, size_t &i, t_location &location);
		bool parseErrorPagesBlock(std::vector<Token> &tokens, size_t &i, std::map<int, std::string> &map);
		bool parseDirective(std::vector<Token> &tokens, size_t &i, const std::string &context, t_server &server, t_location *location);
		void syntaxError(const std::string &msg, int line) const;
};

#endif