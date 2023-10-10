#ifndef CONFIGURATIONPARSER_HPP
#define CONFIGURATIONPARSER_HPP

#include <string>
#include <map>
#include <vector>
#include "RouteConfig.hpp"
#include "ServerConfig.hpp"

struct ParsedRouteConfig {
	std::vector<std::string> methods;
	std::string redirect;
	std::string root;
	bool directory_listing;
	std::string default_file;
	std::string cgi_extension;
	bool allow_file_upload;
	std::string upload_location;
};

struct ParsedServerConfig {
	std::string host;
	int port;
	std::string server_name;
	std::string default_error_page;
	int max_client_body_size;
	std::map<std::string, RouteConfig> routes;
};

class ConfigurationParser {
	private:
		std::string filename;
	public:
		ConfigurationParser(std::string filename);
		~ConfigurationParser();
		std::map<std::string, ParsedServerConfig> parse();

		class InvalidConfigurationException : public std::exception {
			public:
				InvalidConfigurationException(const std::string& message);
				virtual const char* what() const throw();
			private:
				std::string msg_;
		};
};

#endif
