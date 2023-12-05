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

class ConfigurationParser {
	private:
		std::string filename;
	public:
		ConfigurationParser();
		~ConfigurationParser();
		static std::map<std::string, Server> parse(const std::string& filename);

		// Server Parsing
    static void parseServerConfig(std::string& line, Server& serverConfig);
		static void parseHost(std::string& line, Server& serverConfig);
		static void parsePort(std::string& line, Server& serverConfig);
		static void parseServerName(std::string &line, Server& serverConfig);
		static void parseErrorPages(std::string& line, Server& serverConfig);
		static void parseClientMaxBodySize(std::string& line, Server& serverConfig);

		// Route Parsing
    static void parseRouteConfig(std::string& line, Route& routeConfig);
		static void parseRoute(std::string& line, Route& route);
		static void parseMethods(std::string& line, Route& route);
		static void parseRedirect(std::string& line, Route& route);
		static void parseRoot(std::string& line, Route& route);
		static void parseDirectoryListing(std::string& line, Route& route);
		static void parseDefaultFile(std::string& line, Route& route);
		static void parseCgiExtensions(std::string& line, Route& route);
		static void parseAllowFileUpload(std::string& line, Route& route);
		static void parseUploadLocation(std::string& line, Route& route);


		// Exception
		class InvalidConfigurationException : public std::exception {
			public:
				InvalidConfigurationException(const std::string& message);
				virtual const char* what() const throw();
				virtual ~InvalidConfigurationException() throw() {}  // Explicitly declared destructor
			private:
				std::string msg_;
		};
};

#endif
