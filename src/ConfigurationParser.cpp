#include "../inc/ConfigurationParser.hpp"

ConfigurationParser::ConfigurationParser(std::string filename) : filename(filename) {}

ConfigurationParser::~ConfigurationParser() {}

std::map<std::string, ParsedServerConfig> ConfigurationParser::parse() {
	std::map<std::string, ParsedServerConfig> parsedConfigs;
	std::ifstream file(filename.c_str());
	std::string line;
	ParsedServerConfig* currentServerConfig = NULL;
	ParsedRouteConfig* currentRouteConfig = NULL;

	while(std::getline(filem line)) {
		if (line.empty() || line[0] == '#')
			continue; // Skip empty lines and comments
		if (line.find() != std::string::npos) {
			// Extract server name and create a new ParsedServerConfig
			std::string serverName = extractServerName(line);
		}

	}

}
