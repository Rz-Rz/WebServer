#include "Logger.hpp"
#include "ParsingUtils.hpp"
#include "ConfigurationParser.hpp"
#include "Server.hpp"
#include "Route.hpp"
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include <map>
#include <set>
#include <unistd.h>
#include <climits>

ConfigurationParser::InvalidConfigurationException::InvalidConfigurationException(const std::string& message) : msg_(message) {}

const char *ConfigurationParser::InvalidConfigurationException::what() const throw() {
	return msg_.c_str();
}


ConfigurationParser::ConfigurationParser(const std::string& filename) : filename(filename) {}

ConfigurationParser::~ConfigurationParser() {}

std::map<std::string, Server> ConfigurationParser::parse() {
  std::map<std::string, Server> parsedConfigs;
  std::ifstream file(filename.c_str());
  std::string line;
  Server currentServerConfig;
  Route currentRouteConfig;
  bool ParsingServer = false;
  bool ParsingRoute = false;
  ParserState state = START;

  while(std::getline(file, line)) {
    if (line.empty() || line[0] == '#')
      continue; // Skip empty lines and comments
    switch (state) {
      case START:
        if (ParsingUtils::matcher(line, "[server:"))
        {
          state = SERVER_CONFIG;
          if (ParsingServer == true) // save the previous server configuration before starting the new one.
            parsedConfigs.insert(std::pair<std::string, Server>(currentServerConfig.getServerName(), currentServerConfig));
          ParsingServer = true;
        }
        if (ParsingUtils::matcher(line, "[route:"))
        {
          state = ROUTE_CONFIG;
          if (ParsingRoute == true) // save the previous route configuration before starting the new one.
            currentServerConfig.addRoute(currentRouteConfig.getRoutePath(), currentRouteConfig);
          ParsingRoute = true;
        }
        break;
      case SERVER_CONFIG:
        if (ParsingUtils::matcher(line, "[route:"))
          state = ROUTE_CONFIG;
        else {
          if (ParsingUtils::matcher(line, "[server:"))
            ConfigurationParser::parseServerName(line, currentServerConfig);

          if (ParsingUtils::matcher(line, "host"))
            ConfigurationParser::parseHost(line, currentServerConfig);

          if (ParsingUtils::matcher(line, "port"))
            ConfigurationParser::parsePort(line, currentServerConfig);

          if (ParsingUtils::matcher(line, "error_page"))
            ConfigurationParser::parseErrorPages(line, currentServerConfig);

          if (ParsingUtils::matcher(line, "client_max_body_size"))
            ConfigurationParser::parseClientMaxBodySize(line, currentServerConfig);
        }
        break;
      case ROUTE_CONFIG:
        if (ParsingUtils::matcher(line, "[server:"))
        {
          state = SERVER_CONFIG; // save the route to the current server before switching to a new server.
          currentServerConfig.addRoute(currentRouteConfig.getRoutePath(), currentRouteConfig);
        }
        if (ParsingUtils::matcher(line, "[route:"))
        {
          currentServerConfig.addRoute(currentRouteConfig.getRoutePath(), currentRouteConfig); // save the route to the current server before switching to a new route.
          state = ROUTE_CONFIG;
        }

        if (ParsingUtils::matcher(line, "methods"))
          ConfigurationParser::parseMethods(line, currentRouteConfig);

        if (ParsingUtils::matcher(line, "redirect"))
          ConfigurationParser::parseRedirect(line, currentRouteConfig);

        if (ParsingUtils::matcher(line, "root"))
          ConfigurationParser::parseRoot(line, currentRouteConfig);

        if (ParsingUtils::matcher(line, "directory_listing"))
          ConfigurationParser::parseDirectoryListing(line, currentRouteConfig);

        if (ParsingUtils::matcher(line, "default_file"))
          ConfigurationParser::parseDefaultFile(line, currentRouteConfig);

        if (ParsingUtils::matcher(line, "cgi_extensions"))
          ConfigurationParser::parseCgiExtensions(line, currentRouteConfig);

        if (ParsingUtils::matcher(line, "allow_file_upload"))
          ConfigurationParser::parseAllowFileUpload(line, currentRouteConfig);

        if (ParsingUtils::matcher(line, "upload_location"))
          ConfigurationParser::parseUploadLocation(line, currentRouteConfig);
    }
        break;
  }
  return parsedConfigs;
}

// Parse server Config
void ConfigurationParser::parseHost(std::string& line, Server& serverConfig) {
	std::istringstream iss(line);
	std::string host;
	iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
	getline(iss, host);  // Read the rest into value
	if (host.empty()) {
		throw ConfigurationParser::InvalidConfigurationException("Host cannot be empty");
	}
	if (ParsingUtils::isValidIPv4(host) == false)
		throw ConfigurationParser::InvalidConfigurationException("Invalid host: " + host);
	serverConfig.setHost(host);
}

void ConfigurationParser::parsePort(std::string& line, Server& serverConfig) {
	std::istringstream iss(line);
	std::string portStr;
	iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
	getline(iss, portStr);
	if (portStr.empty())
		throw ConfigurationParser::InvalidConfigurationException("Port cannot be empty");
	std::istringstream iss2(portStr);
	int port;
	iss2 >> port;
	if (port < 1 || port > 65535)
		throw ConfigurationParser::InvalidConfigurationException("Invalid port: " + portStr);
	serverConfig.setPort(port);
}

void ConfigurationParser::parseServerName(std::string &line, Server& serverConfig) 
{
	const std::string prefix = "[server:";
	const std::string suffix = "]";
	
	// Check if the line has the correct prefix and suffix
	if (line.substr(0, prefix.length()) != prefix || line.substr(line.length() - suffix.length()) != suffix)
		serverConfig.setServerName("DefaultServerName");;
	std::string serverName = line.substr(prefix.length(), line.length() - (prefix.length() + suffix.length()));
	if (serverName.empty() == true)
		serverConfig.setServerName("DefaultServerName");;
	// Validate the server name according to the rules
	if (serverName.length() > 253 || serverName.find("..") != std::string::npos)
		serverConfig.setServerName("DefaultServerName");;
	serverConfig.setServerName(serverName);
}

void ConfigurationParser::parseErrorPages(std::string& line, Server& serverConfig) {
	std::size_t equalPos = line.find('=');
	if (equalPos == std::string::npos || equalPos + 1 == line.size()) {
		Logger::log(WARNING, "Invalid error_page format, reverting to default.");
		return;
	}

	std::string codesAndPath = line.substr(equalPos + 1);
	std::istringstream iss(codesAndPath);
	std::string token;
	std::vector<std::string> tokens;

	// Split the string by both space and comma
	while (iss >> token) {
		size_t commaPos = token.find(',');
		while (commaPos != std::string::npos) {
			tokens.push_back(token.substr(0, commaPos));
			token = token.substr(commaPos + 1);
			commaPos = token.find(',');
		}
		tokens.push_back(token);
	}

	if (tokens.empty()) {
		Logger::log(WARNING, "error_page configuration is empty, reverting to default.");
		return;
	}

	std::string path = tokens.back(); // The last token is the path
	tokens.pop_back(); // Remove the path from the list of tokens

	if (!ParsingUtils::doesPathExist(path)) {
		Logger::log(WARNING, "error_page path does not exist or is not readable, reverting to default.");
		return;
	}

	// Process each error code
	for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
		const std::string& codeStr = *it;

		bool isDigit = true;
		for (size_t i = 0; i < codeStr.size(); ++i) {
			if (!isdigit(static_cast<unsigned char>(codeStr[i]))) {
				isDigit = false;
				break;
			}
		}

		if (codeStr.empty() || !isDigit) {
			Logger::log(WARNING, "error_page code must be a number, reverting to default.");
			continue;
		}

		int errorCode = std::atoi(codeStr.c_str());
		if (errorCode < 100 || errorCode >= 600) {
			Logger::log(WARNING, "Invalid error_page code: " + codeStr);
			continue;
		}
		serverConfig.setErrorPage(errorCode, path);
	}
	serverConfig.hasCustomErrorPage(true);
}

void ConfigurationParser::parseClientMaxBodySize(std::string& line, Server& serverConfig) {
	std::istringstream iss(line);
	std::string sizeStr;
	iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
	getline(iss, sizeStr);

	if (sizeStr.empty()) {
		Logger::log(WARNING, "client_max_body_size is empty, reverting to default.");
		serverConfig.setMaxClientBodySize(1000000);  // Default value
		return;
	}

	long long size = 0;
	long long multiplier = 1;  // Default is bytes
	size_t i = 0;
	bool invalidCharacterFound = false;

	// Parse the number part
	for (; i < sizeStr.size() && isdigit(sizeStr[i]); ++i) {
		size = size * 10 + (sizeStr[i] - '0');
	}

	// Check for suffix and set the multiplier
	if (i < sizeStr.size()) {
		char suffix = sizeStr[i++];
		if (suffix == 'k' || suffix == 'K') {
			multiplier = 1024;  // KB
		} else if (suffix == 'm' || suffix == 'M') {
			multiplier = 1024 * 1024;  // MB
		} else {
			invalidCharacterFound = true;
		}
	}

	// Check for extra characters or invalid characters
	if (i != sizeStr.size() || invalidCharacterFound) {
		Logger::log(WARNING, "Invalid client_max_body_size value, reverting to default.");
		serverConfig.setMaxClientBodySize(1000000);  // Default value
	} else {
		serverConfig.setMaxClientBodySize(size * multiplier);
	}
}

// Parse route Config
void ConfigurationParser::parseRoute(std::string& line, Route& route) {
    const std::string prefix = "[route:";
    const std::string suffix = "]";

    // Check if the line has the correct prefix and suffix
    if (line.substr(0, prefix.length()) != prefix || line.substr(line.length() - suffix.length()) != suffix)
        throw ConfigurationParser::InvalidConfigurationException("Invalid route name: " + line);

    std::string routeName = line.substr(prefix.length(), line.length() - prefix.length() - suffix.length());
    // Validate the route
    if (routeName.empty()) {
        throw ConfigurationParser::InvalidConfigurationException("Empty route: " + routeName);
    }
    for (std::string::const_iterator it = routeName.begin(); it != routeName.end(); ++it) {
        unsigned char c = static_cast<unsigned char>(*it);
        if (c <= 32 || c == 127) {
            throw ConfigurationParser::InvalidConfigurationException("Invalid route name: " + routeName); // Found a control character
        }
    }
    if (routeName.find("..") != std::string::npos) {
        throw ConfigurationParser::InvalidConfigurationException("Invalid route name: " + routeName); // Found ".."
    }
    if (ParsingUtils::containsIllegalUrlCharacters(routeName)) {
        throw ConfigurationParser::InvalidConfigurationException("Invalid route name: " + routeName); // Found illegal character
    }
    if (routeName[0] != '/') {
      Logger::log(WARNING, "route name must start with a '/', prefixed " + routeName + " with a '/'");
      ParsingUtils::setPrefixString(routeName, "/");
    };
    route.setRoutePath(routeName);
}

void ConfigurationParser::parseMethods(std::string& line, Route& route) {
    std::istringstream iss(line);
    std::string method;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  // Ignore everything until the '='
    while (iss >> method) {
      if (method.empty()) {
        Logger::log(WARNING, "method are empty, GET and POST will be accepted for route " + route.getRoutePath());
        route.setGetMethod(true);
        route.setPostMethod(true);
      }
      if (ParsingUtils::matcher(method, "GET"))
      {
        Logger::log(INFO, "GET method found for route " + route.getRoutePath());
        route.setGetMethod(true);
      }
      else if (ParsingUtils::matcher(method, "POST"))
      {
        Logger::log(INFO, "POST method found for route " + route.getRoutePath());
        route.setPostMethod(true);
      }
      else
        throw ConfigurationParser::InvalidConfigurationException("Invalid method: " + method);
    }
}

void ConfigurationParser::parseRedirect(std::string& line, Route& route) {
    std::istringstream iss(line);
    std::string redirect;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  // Ignore everything until the '='
    getline(iss, redirect);  // Read the rest into value
    if (redirect.empty()) {
	    Logger::log(WARNING, "redirect is empty, no redirect is registered for route " + route.getRoutePath());
      route.setRedirect(false);
	    return;
    }
    if (ParsingUtils::controlCharacters(redirect))
    {
	    Logger::log(WARNING, "Control characters found in redirect: " + redirect + ", no redirect is registered for route " + route.getRoutePath());
      route.setRedirect(false);
	    return;
    }
    ParsingUtils::trimAndLower(redirect);
    if (!ParsingUtils::isAbsoluteUrl(redirect) && redirect[0] != '/') {
	    Logger::log(WARNING, "redirect must start with a '/', prefixed " + redirect + " with a '/'"); 
	    ParsingUtils::setPrefixString(redirect, "/");
    }
    route.setRedirectLocation(redirect);
    route.setRedirect(true);
}

void ConfigurationParser::parseRoot(std::string& line, Route& route) {
    std::istringstream iss(line);
    std::string root;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
    getline(iss, root); 

    if (root.empty()) {
        Logger::log(WARNING, "root is empty, using default root: " + root);
        root = "/var/www/webserver/"; // Default compiled-in path
    }
    // Check for relative paths leading outside of server root
    if (root.find("../") != std::string::npos) {
        throw ConfigurationParser::InvalidConfigurationException("Relative paths outside server root are forbidden: " + root);
    }
    if (ParsingUtils::doesPathExist(root) == false) {
	    Logger::log(WARNING, "root path does not exist: " + root + " reverting to default root.");
	    root = "/var/www/webserver/";
    } else if (ParsingUtils::hasReadPermissions(root) == false) {
	    Logger::log(WARNING, "root path does not have read permissions: " + root + " reverting to default root.");
	    root = "/var/www/webserver/";
    }
    else if (ParsingUtils::hasWritePermissions(root) == false) {
	    Logger::log(WARNING, "root path does not have write permissions: " + root + " reverting to default root.");
	    root = "/var/www/webserver/"; // Default compiled-in path
    }
    route.setRootDirectoryPath(root);
}


void ConfigurationParser::parseDirectoryListing(std::string& line, Route& route) {
    std::istringstream iss(line);
    std::string listing;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
    getline(iss, listing); 

    if (listing.empty()) {
	Logger::log(WARNING, "directory_listing is empty, reverting to default.");
	route.setDirectoryListing(false);
	return;
    }
    if (ParsingUtils::matcher(listing, "on")) {
	Logger::log(INFO, "directory_listing is on for route " + route.getRoutePath());
	route.setDirectoryListing(true);
    } else if (ParsingUtils::matcher(listing, "off")) {
	Logger::log(INFO, "directory_listing is off for route " + route.getRoutePath());
	route.setDirectoryListing(false);
    } else {
	Logger::log(WARNING, "Invalid directory_listing value: " + listing + ", reverting to default (false).");
	route.setDirectoryListing(false);
    }
}

void ConfigurationParser::parseDefaultFile(std::string& line, Route& route) {
    std::istringstream iss(line);
    std::string file;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
    getline(iss, file); 

    if (file.empty()) {
	Logger::log(WARNING, "default_file is empty, reverting to default.");
	route.setDefaultFile("index.html");
	return;
    }
    if (ParsingUtils::controlCharacters(file)) {
	Logger::log(WARNING, "Control characters found in default_file: " + file + ", reverting to default.");
	route.setDefaultFile("index.html");
	return;
    }
    if (file[0] != '/') {
	Logger::log(WARNING, "default_file must start with a '/', prefixed " + file + " with a '/'"); 
	ParsingUtils::setPrefixString(file, "/");
    }
    route.setDefaultFile(file);
}

void ConfigurationParser::parseCgiExtensions(std::string& line, Route& route) {
    std::istringstream iss(line);
    std::string extensions;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
    getline(iss, extensions); 

    if (extensions.empty()) {
	Logger::log(WARNING, "cgi_extensions is empty, reverting to default.");
	route.setHasCGI(false);
	return;
    }
    std::istringstream iss2(extensions);
    std::string extension;
    std::vector<std::string> extensionsList;
    while (iss2 >> extension) {
	if (extension.empty()) {
	    Logger::log(WARNING, "Empty extension found in cgi_extensions, reverting to default (false).");
	    route.setHasCGI(false);
	    return;
	}
	if (ParsingUtils::controlCharacters(extension)) {
	    Logger::log(WARNING, "Control characters found in cgi_extensions: " + extension + ", reverting to default (false).");
	    route.setHasCGI(false);
	    return;
	}
	if (extension[0] != '.') {
	    Logger::log(WARNING, "Extension must start with a '.', prefixed " + extension + " with a '.'"); 
	    ParsingUtils::setPrefixString(extension, ".");
	}
	extensionsList.push_back(extension);
    }
    route.setCgiExtensions(extensionsList);
}

void ConfigurationParser::parseAllowFileUpload(std::string& line, Route& route) {
    std::istringstream iss(line);
    std::string allow;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
    getline(iss, allow); 

    if (allow.empty()) {
	Logger::log(WARNING, "allow_file_upload is empty, reverting to default.");
	route.setAllowFileUpload(false);
	return;
    }
    if (ParsingUtils::matcher(allow, "on")) {
	Logger::log(INFO, "allow_file_upload is on for route " + route.getRoutePath());
	route.setAllowFileUpload(true);
    } else if (ParsingUtils::matcher(allow, "off")) {
	Logger::log(INFO, "allow_file_upload is off for route " + route.getRoutePath());
	route.setAllowFileUpload(false);
    } else {
	Logger::log(WARNING, "Invalid allow_file_upload value: " + allow + ", reverting to default (false).");
	route.setAllowFileUpload(false);
    }
}

void ConfigurationParser::parseUploadLocation(std::string& line, Route& route) {
    std::istringstream iss(line);
    std::string location;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
    getline(iss, location); 

    if (location.empty()) {
	Logger::log(WARNING, "upload_location is empty, reverting to default.");
	route.setUploadLocation("/var/www/webserver/uploads/");
	return;
    }
    if (ParsingUtils::controlCharacters(location)) {
	Logger::log(WARNING, "Control characters found in upload_location: " + location + ", reverting to default.");
	route.setUploadLocation("/var/www/webserver/uploads/");
	return;
    }
    if (location[0] != '/') {
	Logger::log(WARNING, "upload_location must start with a '/', prefixed " + location + " with a '/'"); 
	ParsingUtils::setPrefixString(location, "/");
    }
    route.setUploadLocation(location);
}

std::string getCurrentExecutablePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string path(result, (count > 0) ? count : 0);

    // If you want just the directory, you can do:
    size_t found = path.find_last_of("/\\");
    return path.substr(0, found);
}
