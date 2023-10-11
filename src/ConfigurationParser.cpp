#include "../inc/ConfigurationParser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <limits>
#include <unistd.h>

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
				throw ConfigurationParser::InvalidConfigurationException("Default error page path cannot be empty");
			if (pathExists(errorPagePath) == false)
				throw ConfigurationParser::InvalidConfigurationException("Default error page path does not exist or does not have read permissions: " + errorPagePath);
			// currentServerConfig->default_error_page = errorPagePath;
		}

		if (line.find("[route:") != std::string::npos) {
			if (ParsingServer == true)
				ParsingServer == false;
			currentServerConfig->server_name = extractServerName(line);
			ParsingServer = true;
		}

	}
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

    if (suffix == 'k' || suffix == 'K') {
        multiplier = 1024;  // KB
    } else if (suffix == 'm' || suffix == 'M') {
        multiplier = 1024 * 1024;  // MB
    } else if (!std::isdigit(suffix)) {
        throw ConfigurationParser::InvalidConfigurationException("Invalid max_client_body_size suffix: " + input);
    }
    long long value = std::stoll(input);  // might throw std::invalid_argument or std::out_of_range
    return value * multiplier;
}
