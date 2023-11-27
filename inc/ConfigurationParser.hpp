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
#include "Route.hpp"
#include "Server.hpp"

enum ParserState {
    START,
    SERVER_CONFIG,
    ROUTE_CONFIG,
};

class ConfigurationParser {
	private:
		std::string filename;
	public:
		ConfigurationParser(const std::string& filename);
		~ConfigurationParser();
		std::map<std::string, Server> parse();

		// Server Parsing
		void parseHost(std::string& line, Server& serverConfig);
		void parsePort(std::string& line, Server& serverConfig);
		void parseServerName(std::string &line, Server& serverConfig);
		void parseErrorPages(std::string& line, Server& serverConfig);
		void parseClientMaxBodySize(std::string& line, Server& serverConfig);

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
