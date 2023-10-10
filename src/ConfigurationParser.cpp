#include "../inc/ConfigurationParser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

ConfigurationParser::InvalidConfigurationException::InvalidConfigurationException(const std::string& message) : msg_(message) {}

const char *ConfigurationParser::InvalidConfigurationException::what() const throw() {
	return msg_.c_str();
}

std::string extractServerName(std::string line) {
	const std::string prefix = "[server:";
	const std::string suffix = "]";

	// Check if the line has the correct prefix and suffix
	if (line.substr(0, prefix.length()) != prefix || line.substr(line.length() - suffix.length()) != suffix) {
		throw ConfigurationParser::InvalidConfigurationException("Malformed server configuration line: " + line);
	}

	std::string serverName = line.substr(prefix.length(), line.length() - prefix.length() - suffix.length());

	if (serverName.empty()) {
		throw ConfigurationParser::InvalidConfigurationException("Server name cannot be empty");
	}

	// Validate the server name according to the rules
    if (serverName.length() > 253 || serverName.find("..") != std::string::npos) {
        throw ConfigurationParser::InvalidConfigurationException("Invalid server name: " + serverName);
    }

	return serverName;
}

ConfigurationParser::ConfigurationParser(std::string filename) : filename(filename) {}

ConfigurationParser::~ConfigurationParser() {}

std::map<std::string, ParsedServerConfig> ConfigurationParser::parse() {
	std::map<std::string, ParsedServerConfig> parsedConfigs;
	std::ifstream file(filename.c_str());
	std::string line;
	ParsedServerConfig* currentServerConfig = NULL;
	ParsedRouteConfig* currentRouteConfig = NULL;

	while(std::getline(file, line)) {
		if (line.empty() || line[0] == '#')
			continue; // Skip empty lines and comments
		if (line.find("[server:") != std::string::npos) {
			// Extract server name and create a new ParsedServerConfig
			std::string serverName = extractServerName(line);
		}
	}
}
