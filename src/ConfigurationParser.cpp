#include "../inc/Logger.hpp"
#include "../inc/ConfigurationParser.hpp"
#include <iostream>     // std::cout
#include <sstream>      // std::istringstream
#include <string>       // std::string
#include <limits>
#include <map>
#include <set>
#include <unistd.h>
#include <climits>

ConfigurationParser::InvalidConfigurationException::InvalidConfigurationException(const std::string& message) : msg_(message) {}

const char *ConfigurationParser::InvalidConfigurationException::what() const throw() {
	return msg_.c_str();
}

std::string extractServerName(std::string line) 
{
	const std::string prefix = "[server:";
	const std::string suffix = "]";

	// Check if the line has the correct prefix and suffix
	if (line.substr(0, prefix.length()) != prefix || line.substr(line.length() - suffix.length()) != suffix)
		return "DefaultServerName";
	std::string serverName = line.substr(prefix.length(), line.length() - suffix.length());
	// Validate the server name according to the rules
    if (serverName.length() > 253 || serverName.find("..") != std::string::npos)
		return "DefaultServerName";
	return serverName;
}

ConfigurationParser::ConfigurationParser(const std::string& filename) : filename(filename) {}

ConfigurationParser::~ConfigurationParser() {}

std::map<std::string, ParsedServerConfig> ConfigurationParser::parse() {
	std::map<std::string, ParsedServerConfig> parsedConfigs;
	std::ifstream file(filename.c_str());
	std::string line;
	ParsedServerConfig currentServerConfig;
	ParsedRouteConfig currentRouteConfig;
	bool ParsingServer = false;
	bool ParsingRoute = false;

	while(std::getline(file, line)) {
		if (line.empty() || line[0] == '#')
			continue; // Skip empty lines and comments

		if (line.find("[server:") != std::string::npos) {
			if (ParsingServer == true)
				parsedConfigs[currentServerConfig.server_name] = currentServerConfig;
			currentServerConfig.server_name = extractServerName(line);
			ParsingServer = true;
		}

		if (line.find("host") != std::string::npos) {
			std::istringstream iss(line);
			std::string host;
			iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
			getline(iss, host);  // Read the rest into value
			if (host.empty()) {
				throw ConfigurationParser::InvalidConfigurationException("Host cannot be empty");
			}
			if (isValidIPv4(host) == false)
				throw ConfigurationParser::InvalidConfigurationException("Invalid host: " + host);
			currentServerConfig.host = host;
		}
		
		if (line.find("port") != std::string::npos) {
			std::istringstream iss(line);
			std::string portStr;
			iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
			getline(iss, portStr);
			if (portStr.empty())
				throw ConfigurationParser::InvalidConfigurationException("Port cannot be empty");
			int port = std::stoi(portStr);  // Convert string to integer
			if (port < 1 || port > 65535)
				throw ConfigurationParser::InvalidConfigurationException("Invalid port: " + portStr);
			currentServerConfig.port = port;
		}

		if (line.find("default_error_page") != std::string::npos) {
			std::istringstream iss(line);
			std::string errorPagePath;
			iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
			getline(iss, errorPagePath);
			if (errorPagePath.empty())
				Logger::log(WARNING, "default_error_page path is empty, reverting to default.");
			if (pathExists(errorPagePath) == false)
				Logger::log(WARNING, "default_error_page path does not exist or is not readable, reverting to default.");
			// currentServerConfig->default_error_page = errorPagePath;
		}

		if (line.find("[route:") != std::string::npos) {
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

		if (line.find("methods") != std::string::npos) {
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
		if (line.find("root") != std::string::npos)
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
    int val = std::atoi(item.c_str());
    if (val < 0 || val > 255) {
      return false;
    }
  }
  return true;
}

bool pathExists(const std::string& path) {
  return access(path.c_str(), F_OK | R_OK) == 0;
}

long long parseMaxBodySize(const std::string& input) {
  char suffix = input.back();
  long long multiplier = 1;  // default is bytes

  if (suffix == 'k' || suffix == 'K')
    multiplier = 1024;  // KB
  else if (suffix == 'm' || suffix == 'M')
    multiplier = 1024 * 1024;  // MB
  else if (!std::isdigit(suffix)) 
    throw ConfigurationParser::InvalidConfigurationException("Invalid max_client_body_size suffix: " + input);

  long long value = std::stoll(input);  // might throw std::invalid_argument or std::out_of_range
  return value * multiplier;
}
