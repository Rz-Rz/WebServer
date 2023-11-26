#ifndef CONFIGURATIONPARSER_HPP
#define CONFIGURATIONPARSER_HPP

#include <iostream>     // std::cout
#include <sstream>      // std::istringstream
#include <string>       // std::string
#include <limits>
#include <map>
#include <set>
#include <unistd.h>
#include <climits>
#include <string>
#include <vector>

enum ParserState {
    START,
    SERVER_CONFIG,
    ROUTE_CONFIG,
};



struct ParsedRouteConfig {
  std::string route_path;
	std::set<std::string> methods; // changed from vector to set for efficient lookups
	std::string redirect;
	std::string root_directory_path; // the root from which to look for files
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
	bool error_pages_set; // Flag to check if error pages are set
	long long max_client_body_size; // changed to size_t for larger sizes
	bool max_client_body_size_set; // Flag to check if max_client_body_size is set
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

		// Server Parsing
		void parseHost(std::string& line, ParsedServerConfig& serverConfig);
		void parsePort(std::string& line, ParsedServerConfig& serverConfig);
		std::string parseServerName(std::string& line);
		void parseErrorPages(std::string& line, ParsedServerConfig& serverConfig);
		void parseMaxClientBodySize(std::string& line, ParsedServerConfig& serverConfig);

		// Route Parsing


		class InvalidConfigurationException : public std::exception {
			public:
				InvalidConfigurationException(const std::string& message);
				virtual const char* what() const throw();
				virtual ~InvalidConfigurationException() throw() {}  // Explicitly declared destructor
			private:
				std::string msg_;
		};
};

// parse the server name
std::string extractServerName(std::string line);
bool isValidIPv4(const std::string& host);
bool pathExists(const std::string& path);
long long parseMaxBodySize(const std::string& input);
bool isValidMethod(const std::string& method);
bool isValidRoute(const std::string& route);
std::string getCurrentExecutablePath();
bool containsInvalidCharacter(const std::string& str);
bool isValidRedirect(const std::string& url);
bool caseInsensitiveFind(const std::string& str, const std::string& toFind);
std::string toLower(const std::string& str);

#endif
