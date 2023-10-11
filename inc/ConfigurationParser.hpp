#ifndef CONFIGURATIONPARSER_HPP
#define CONFIGURATIONPARSER_HPP

#include <string>
#include <map>
#include <vector>
#include "RouteConfig.hpp"
#include "ServerConfig.hpp"

struct ParsedRouteConfig {
	std::set<std::string> methods; // changed from vector to set for efficient lookups
	std::string redirect;
	std::string root_directory_path; // more verbose
	bool directory_listing;
	std::string default_file;
	std::vector<std::string> cgi_extensions; // changed to vector to handle multiple extensions
	bool allow_file_upload;
	std::string upload_location;

	ParsedRouteConfig()
		: directory_listing(false), allow_file_upload(false) {}
};

struct ParsedServerConfig {
	std::string host;
	int port;
	std::string server_name;
	std::map<int, std::string> error_pages; // Map to hold different error pages for different codes
	long long max_client_body_size; // changed to size_t for larger sizes
	std::map<std::string, ParsedRouteConfig> routes; // fixed the typo `RouteConfig` to `ParsedRouteConfig`

	ParsedServerConfig()
		: port(0), max_client_body_size(0) {}
};

class ConfigurationParser {
	private:
		std::string filename;
	public:
		ConfigurationParser(const std::string& filename);
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

// parse the server name
std::string extractServerName(std::string line);
bool isValidIPv4(const std::string& host);
bool pathExists(const std::string& path);
long long parseMaxBodySize(const std::string& input);

#endif
