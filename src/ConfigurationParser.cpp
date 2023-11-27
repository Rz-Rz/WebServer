#include "Logger.hpp"
#include "ParsingUtils.hpp"
#include "ConfigurationParser.hpp"
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
				if (ParsingUtils::matcher(line, "[server:")) {
					if (ParsingServer == true)
						parsedConfigs[currentServerConfig.server_name] = currentServerConfig;
					currentServerConfig.server_name = ConfigurationParser::parseServerName(line);
					ParsingServer = true;
					state = SERVER_CONFIG;
				} else if (ParsingUtils::matcher(line, "[route:")) {
					if (ParsingRoute == true)
						currentServerConfig.routes[currentRouteConfig.route_path] = currentRouteConfig; //save the previous route
					ParsingRoute = true;
					const std::string prefix = "[route:";
					const std::string suffix = "]";

					// Check if the line has the correct prefix and suffix
					if (line.substr(0, prefix.length()) != prefix || line.substr(line.length() - suffix.length()) != suffix)
						throw ConfigurationParser::InvalidConfigurationException("Invalid route name: " + line);
					std::string routeName = line.substr(prefix.length(), line.length() - suffix.length());
					if (isValidRoute(routeName) == false)
						throw ConfigurationParser::InvalidConfigurationException("Invalid route name: " + routeName);
					currentRouteConfig.route_path = routeName;
					state = ROUTE_CONFIG;
				}
				break;

			case SERVER_CONFIG:
				if (ParsingUtils::matcher(line, "[route:")) {
					parsedConfigs[currentServerConfig.server_name] = currentServerConfig;
					state = ROUTE_CONFIG;
					// should I initialize the new route config here ?
				} else {

					if (ParsingUtils::matcher(line, "host"))
						ConfigurationParser::parseHost(line, currentServerConfig);

					if (ParsingUtils::matcher(line, "port"))
						ConfigurationParser::parsePort(line, currentServerConfig);

					if (ParsingUtils::matcher(line, "error_page"))
						ConfigurationParser::parseErrorPages(line, currentServerConfig);

					if (ParsingUtils::matcher(line, "client_max_body_size"))
						ConfigurationParser::parseClientMaxBodySize(line, currentServerConfig);
				}

			case ROUTE_CONFIG:
				if (ParsingUtils::matcher(line, "[server:]")) {

				}

}
		if (ParsingUtils::matcher(line, "[route:")) {
			if (ParsingRoute == true)
				currentServerConfig.routes[currentRouteConfig.route_path] = currentRouteConfig;
			ParsingRoute = true;
			const std::string prefix = "[server:";
			const std::string suffix = "]";

			// Check if the line has the correct prefix and suffix
			if (line.substr(0, prefix.length()) != prefix || line.substr(line.length() - suffix.length()) != suffix)
				throw ConfigurationParser::InvalidConfigurationException("Invalid route name: " + line);
			std::string routeName = line.substr(prefix.length(), line.length() - suffix.length());
			if (isValidRoute(routeName) == false)
				throw ConfigurationParser::InvalidConfigurationException("Invalid route name: " + routeName);
			currentRouteConfig.route_path = routeName;
		}


		if (ParsingUtils::matcher(line, "error_page")) {
		}

		if (ParsingUtils::matcher(line, "methods")) {
			std::istringstream iss(line);
			std::string method;
			iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
			getline(iss, method);
			if (method.empty())
			{
				Logger::log(WARNING, "method are empty, all method will be accepted.");
				currentRouteConfig.methods.insert("GET");
				currentRouteConfig.methods.insert("HEAD");
			}
			if (isValidMethod(method) == false)
				throw ConfigurationParser::InvalidConfigurationException("Invalid method: " + method);
			currentRouteConfig.methods.insert(method);
		}

		if (ParsingUtils::matcher(line, "root"))
		{
			std::istringstream iss(line);
			std::string root;
			iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
			getline(iss, root);
			if (root.empty())
			{
				Logger::log(WARNING, "PARSING: root path is empty, reverting to default root.");
				root = getCurrentExecutablePath();
			}
			if (pathExists(root) == false)
				throw ConfigurationParser::InvalidConfigurationException("Root path does not exist or is not readable");
			currentRouteConfig.root_directory_path = root;
		}
	}
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
	if (isValidIPv4(host) == false)
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

	if (!pathExists(path)) {
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

void parseClientMaxBodySize(const std::string& line, Server& serverConfig) {
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





bool containsInvalidCharacter(const std::string& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        // Check if character is not alphanumeric and not in the list of safe characters
        if (!isalnum(c) && std::string(":/?&=.#_-").find(c) == std::string::npos) {
            return true;
        }
    }
    return false;
}

bool isValidRedirect(const std::string& url) {
    // Check starting pattern
    if (url.find("/") == 0 || url.find("http://") == 0 || url.find("https://") == 0) {
        if (containsInvalidCharacter(url)) {
            return false;
        }
        return true;
    }
    return false;
}

std::string getCurrentExecutablePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string path(result, (count > 0) ? count : 0);

    // If you want just the directory, you can do:
    size_t found = path.find_last_of("/\\");
    return path.substr(0, found);
}

bool isValidMethod(const std::string& method) {
	if (method.find(",") != std::string::npos)
	{
		size_t pos = method.find(",");
		std::string first = method.substr(0, pos);
		if (first != "GET" && first != "POST")
			return false;
		std::string second = method.substr(pos + 1);
		if (second != "GET" && second != "POST")
			return false;
		return true;
	}
	else 
		if (method != "GET" && method != "POST")
			return false;
	return true;
}

bool isValidRoute(const std::string& route) {
  if (route.empty())
    return false;
  if (route[0] != '/')
    return false;
  for (std::string::const_iterator it = route.begin(); it != route.end(); ++it) {
    unsigned char c = static_cast<unsigned char>(*it);
    if (c <= 32 || c == 127) {
      return false;  // Found a control character
    }
  }
  return true; 
}

bool isValidIPv4(const std::string& host) {
  std::stringstream ss(host);
  std::string item;
  std::vector<std::string> tokens;

  // Splitting at dots
  while (std::getline(ss, item, '.')) {
    tokens.push_back(item);
  }
  // Ensure we have exactly four parts.
  if (tokens.size() != 4) {
    return false;
  }
  for (size_t i = 0; i < tokens.size(); i++) {
    item = tokens[i];
    // Ensure no part has leading zeros.
    if (item.size() > 1 && item[0] == '0') {
      return false;
    }
    // Each part should be a number between 0 and 255.
    for (size_t j = 0; j < item.size(); j++) {
      if (!isdigit(item[j])) {
        return false; // Not a number
      }
    }
    int val = atoi(item.c_str());
    if (val < 0 || val > 255) {
      return false;
    }
  }
  return true;
}

bool pathExists(const std::string& path) {
  return access(path.c_str(), F_OK | R_OK) == 0;
}

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

bool caseInsensitiveFind(const std::string& str, const std::string& toFind) {
    std::string lowerStr = toLower(str);
    std::string lowerToFind = toLower(toFind);
    return lowerStr.find(lowerToFind) != std::string::npos;
}
