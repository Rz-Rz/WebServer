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

ConfigurationParser::ConfigurationParser() {}

ConfigurationParser::~ConfigurationParser() {}

std::map<std::string, Server*> ConfigurationParser::parse(const std::string& filename) {
  std::map<std::string, Server*> parsedConfigs;
  std::ifstream file(filename.c_str());
  std::string line;
  Server* currentServerConfig = new Server();
  Route currentRouteConfig;
  bool isParsingServer = false;
  bool isParsingRoute = false;

  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#')
      continue; // Skip empty lines and comments

    if (ParsingUtils::simpleMatcher(line, "[server:")) {
      if (isParsingServer) {
        if (isParsingRoute) {
          // Save the previously parsed route configuration
          currentServerConfig->addRoute(currentRouteConfig.getRoutePath(), currentRouteConfig);
          currentRouteConfig = Route();
          isParsingRoute = false;
        }
        // Save the previously parsed server configuration
	if (!(parsedConfigs.insert(std::make_pair(currentServerConfig->getServerName(), currentServerConfig)).second)) {
		Logger::log(ERROR, "Duplicate server name: " + currentServerConfig->getServerName());
		delete currentServerConfig; // Important to avoid memory leak
	}
        currentServerConfig = new Server(); // Create a new Server object for the next server
      }
      isParsingServer = true;
      ConfigurationParser::parseServerName(line, *currentServerConfig);
      continue;
    }

    if (ParsingUtils::simpleMatcher(line, "[route:")) {
      if (isParsingRoute) {
        // Save the previously parsed route configuration
        currentServerConfig->addRoute(currentRouteConfig.getRoutePath(), currentRouteConfig);
        currentRouteConfig = Route();
      }
      isParsingRoute = true;
      ConfigurationParser::parseRoute(line, currentRouteConfig);
      continue;
    }
    if (isParsingServer)
      parseServerConfig(line, *currentServerConfig);
    if (isParsingRoute)
      parseRouteConfig(line, currentRouteConfig);
  }
  // Finalize the last parsed route and server
  if (isParsingRoute) {
    currentServerConfig->addRoute(currentRouteConfig.getRoutePath(), currentRouteConfig);
  }
  if (isParsingServer) {
	  if (!(parsedConfigs.insert(std::make_pair(currentServerConfig->getServerName(), currentServerConfig)).second)) {
		  Logger::log(ERROR, "Duplicate server name: " + currentServerConfig->getServerName());
		  delete currentServerConfig; // Important to avoid memory leak
	  }
  } else {
    delete currentServerConfig; // Delete if not used
  }

  return parsedConfigs;
}

void ConfigurationParser::checkValidity(const std::map<std::string, Server*>& servers) {
    // Check if there are no servers defined
    if (servers.empty()) {
        throw ConfigurationParser::InvalidConfigurationException("No servers defined in the configuration");
    }

    // Check for duplicate server names, ports, and routes
    ConfigurationParser::checkForDuplicateServerNames(servers);
    ConfigurationParser::checkForDuplicatePorts(servers);
    ConfigurationParser::checkForDuplicateRoutes(servers);

    // Check for the absence of routes in each server
    for (std::map<std::string, Server*>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->second->getRoutes().empty()) {
            throw ConfigurationParser::InvalidConfigurationException("No routes defined for server: " + it->first);
        }
    }
}

void ConfigurationParser::parseServerConfig(std::string& line, Server& serverConfig) {
  if (ParsingUtils::matcher(line, "host"))
    ConfigurationParser::parseHost(line, serverConfig);

  else if (ParsingUtils::matcher(line, "port"))
    ConfigurationParser::parsePort(line, serverConfig);

  else if (ParsingUtils::matcher(line, "error_page"))
    ConfigurationParser::parseErrorPages(line, serverConfig);

  else if (ParsingUtils::matcher(line, "client_max_body_size"))
    ConfigurationParser::parseClientMaxBodySize(line, serverConfig);
}

void ConfigurationParser::parseRouteConfig(std::string& line, Route& routeConfig) {
  if (ParsingUtils::matcher(line, "methods"))
    ConfigurationParser::parseMethods(line, routeConfig);

  else if (ParsingUtils::matcher(line, "redirect"))
    ConfigurationParser::parseRedirect(line, routeConfig);

  else if (ParsingUtils::matcher(line, "root_directory"))
    ConfigurationParser::parseRoot(line, routeConfig);

  else if (ParsingUtils::matcher(line, "directory_listing"))
    ConfigurationParser::parseDirectoryListing(line, routeConfig);

  else if (ParsingUtils::matcher(line, "default_file"))
    ConfigurationParser::parseDefaultFile(line, routeConfig);

  else if (ParsingUtils::matcher(line, "cgi_extensions"))
    ConfigurationParser::parseCgiExtensions(line, routeConfig);

  else if (ParsingUtils::matcher(line, "allow_file_upload"))
    ConfigurationParser::parseAllowFileUpload(line, routeConfig);

  else if (ParsingUtils::matcher(line, "upload_location"))
    ConfigurationParser::parseUploadLocation(line, routeConfig);

  else if (ParsingUtils::matcher(line, "cgi_pass"))
    ConfigurationParser::parseCgiPass(line, routeConfig);
}

// Parse server Config
void ConfigurationParser::parseHost(std::string& line, Server& serverConfig) {
  std::istringstream iss(line);
  std::string host;
  iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');  
  getline(iss, host);  // Read the rest into value
  if (host.empty())
    return;
  ParsingUtils::trimAndLower(host);
  if (ParsingUtils::isValidIPv4(host) == false)
  {
    Logger::log(WARNING, "Invalid host: " + host + ", reverting to default.");
    return;
  }
  Logger::log(INFO, "Host: " + host);
  serverConfig.setHost(host);
}

void ConfigurationParser::parsePort(std::string& line, Server& serverConfig) {
    std::istringstream iss(line);
    std::string portsStr;
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
    getline(iss, portsStr);
    if (portsStr.empty())
        throw ConfigurationParser::InvalidConfigurationException("Port cannot be empty");

    std::vector<int> ports;
    std::stringstream ss(portsStr);
    std::string portStr;
    while (getline(ss, portStr, ',')) {
        ParsingUtils::trimAndLower(portStr);
        if (ParsingUtils::containsAlpha(portStr))
            throw ConfigurationParser::InvalidConfigurationException("Port cannot contain alpha characters");

        int port;
        std::istringstream issPort(portStr);
        issPort >> port;
        if (port < 1 || port > 65535)
            throw ConfigurationParser::InvalidConfigurationException("Invalid port: " + portStr);

        ports.push_back(port);
    }

    serverConfig.setPorts(ports);
    Logger::log(INFO, "Ports: " + serverConfig.getPortsString());
}

void ConfigurationParser::parseServerName(std::string &line, Server& serverConfig) 
{
  const std::string prefix = "[server:";
  const std::string suffix = "]";

  if (line[0] != '[' || line[line.length() - 1] != ']')
    throw::ConfigurationParser::InvalidConfigurationException("Invalid server name: " + line);
  std::string serverName = line.substr(prefix.length(), line.length() - (prefix.length() + suffix.length()));
  ParsingUtils::trim(serverName);  // Trim leading/trailing spaces
  if (serverName.empty() == true)
    throw::ConfigurationParser::InvalidConfigurationException("Invalid server name: " + line);
  // Validate the server name according to the rules
  if (serverName.length() > 253 || serverName.find("..") != std::string::npos)
    throw::ConfigurationParser::InvalidConfigurationException("Invalid server name: " + serverName);
  Logger::log(INFO, "Server name: " + serverName);
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
  std::string absolutePath = ParsingUtils::getCurrentWorkingDirectory() + path;
  tokens.pop_back(); // Remove the path from the list of tokens

  if (!ParsingUtils::doesPathExist(absolutePath)) {
    Logger::log(WARNING, "error_page path does not exist or is not readable, reverting to default.");
    return;
  }

  if (!ParsingUtils::hasReadPermissions(absolutePath)) {
    Logger::log(WARNING, "error_page path does not have read permissions, reverting to default.");
    return;
  }

  if (!ParsingUtils::isRegularFile(absolutePath)) {
    Logger::log(WARNING, "error_page path is not a regular file, reverting to default.");
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
    Logger::log(INFO, "Error code: " + codeStr + " Error path: " + path);
    serverConfig.setErrorPage(errorCode, absolutePath);
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
    return;
  }

  char* end;
  errno = 0;
  long long size = std::strtol(sizeStr.c_str(), &end, 10);

  if (errno == ERANGE || size < 0 || end == sizeStr.c_str()) {
    Logger::log(WARNING, "client_max_body_size is not a valid number, reverting to default.");
    return;
  }

  long long multiplier = 1;  // Default is bytes
  if (*end != '\0') { // Suffix present
    switch (*end) {
      case 'k':
      case 'K':
        multiplier = 1024;
        break;
      case 'm':
      case 'M':
        multiplier = 1024 * 1024;
        break;
      default:
        Logger::log(WARNING, "Invalid client_max_body_size value, reverting to default.");
        return;
    }
  }

  if (size > LLONG_MAX / multiplier) {
    Logger::log(WARNING, "client_max_body_size calculation overflow, reverting to default.");
    return;
  }

  size *= multiplier;
  const long long maxSize = 100LL * 1024 * 1024;  // 100 MB in bytes
  if (size > maxSize) {
    Logger::log(WARNING, "client_max_body_size exceeds 100 MB limit, reverting to default.");
    return;
  }
  Logger::log(INFO, "client_max_body_size: " + sizeStr);
  serverConfig.setMaxClientBodySize(size);
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
  // ParsingUtils::trim(routeName);
  Logger::log(INFO, "Route name: " + routeName);
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
    if (ParsingUtils::matcher(method, "POST"))
    {
      Logger::log(INFO, "POST method found for route " + route.getRoutePath());
      route.setPostMethod(true);
    }
    if (ParsingUtils::matcher(method, "DELETE"))
    {
      Logger::log(INFO, "DELETE method found for route " + route.getRoutePath());
      route.setDeleteMethod(true);
    }
    if (!route.getGetMethod() && !route.getPostMethod() && !route.getDeleteMethod())
    {
      Logger::log(ERROR, "Invalid method: " + method);
      throw ConfigurationParser::InvalidConfigurationException("Invalid method: " + method);
    }
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
  Logger::log(INFO, "Redirect: " + redirect + " for route " + route.getRoutePath());
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
    return;
  }
  // Check for relative paths leading outside of server root
  if (root.find("../") != std::string::npos) {
    throw ConfigurationParser::InvalidConfigurationException("Relative paths outside server root are forbidden: " + root);
  }
  std::string fullPath = ParsingUtils::getWebservRoot() + root;
  if (ParsingUtils::doesPathExist(fullPath) == false) {
    Logger::log(WARNING, "root path does not exist: " + fullPath + " reverting to default root.");
    return;
  } else if (ParsingUtils::hasReadPermissions(fullPath) == false) {
    Logger::log(WARNING, "root path does not have read permissions: " + fullPath + " reverting to default root.");
    return;
  }
  else if (ParsingUtils::hasWritePermissions(fullPath) == false) {
    Logger::log(WARNING, "root path does not have write permissions: " + fullPath + " reverting to default root.");
    return;
  }
  Logger::log(INFO, "Root: " + fullPath + " for route " + route.getRoutePath());
  route.setRootDirectoryPath(fullPath);
  route.setHasRootDirectoryPath(true);
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
    Logger::log(WARNING, "Invalid directory_listing value: " + listing + ", reverting to default (false) for route " + route.getRoutePath() + ".");
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
    return;
  }
  if (ParsingUtils::controlCharacters(file)) {
    Logger::log(WARNING, "Control characters found in default_file: " + file + ", reverting to default.");
    return;
  }
  if (file[0] != '/') {
    Logger::log(WARNING, "default_file must start with a '/', prefixed " + file + " with a '/'"); 
    ParsingUtils::setPrefixString(file, "/");
  }
  Logger::log(INFO, "Default file: " + file + " for route " + route.getRoutePath());
  route.setDefaultFile(file);
  route.setHasDefaultFile(true);
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
      return;
    }
    if (ParsingUtils::controlCharacters(extension)) {
      Logger::log(WARNING, "Control characters found in cgi_extensions: " + extension + ", reverting to default (false).");
      return;
    }
    if (extension[0] != '.') {
      Logger::log(WARNING, "Extension must start with a '.', prefixed " + extension + " with a '.'"); 
      ParsingUtils::setPrefixString(extension, ".");
    }
    extensionsList.push_back(extension);
  }
  Logger::log(INFO, "CGI extensions: " + extensions + " for route " + route.getRoutePath());
  route.setHasCGI(true);
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
    return;
  }
  if (ParsingUtils::controlCharacters(location)) {
    Logger::log(ERROR, "Control characters found in upload_location: " + location + ", reverting to default.");
    throw ConfigurationParser::InvalidConfigurationException("Control characters found in upload_location: " + location);
  }
  ParsingUtils::trimAndLower(location);
  if (location[0] != '/') {
    Logger::log(WARNING, "upload_location must start with a '/', prefixed " + location + " with a '/'"); 
    ParsingUtils::setPrefixString(location, "/");
  }
  Logger::log(INFO, "Upload location: " + location + " for route " + route.getRoutePath());
  route.setUploadLocation(location);
}

void ConfigurationParser::parseMaxBodySize(std::string& line, Route& route) {
  std::istringstream iss(line);
  std::string sizeStr;
  iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
  getline(iss, sizeStr);

  if (sizeStr.empty()) {
    Logger::log(WARNING, "client_max_body_size is empty, reverting to default.");
    return;
  }

  char* end;
  errno = 0;
  long long size = std::strtol(sizeStr.c_str(), &end, 10);

  if (errno == ERANGE || size < 0 || end == sizeStr.c_str()) {
    Logger::log(WARNING, "client_max_body_size is not a valid number, reverting to default.");
    return;
  }

  long long multiplier = 1;  // Default is bytes
  if (*end != '\0') { // Suffix present
    switch (*end) {
      case 'k':
      case 'K':
        multiplier = 1024;
        break;
      case 'm':
      case 'M':
        multiplier = 1024 * 1024;
        break;
      default:
        Logger::log(WARNING, "Invalid client_max_body_size value, reverting to default.");
        return;
    }
  }

  if (size > LLONG_MAX / multiplier) {
    Logger::log(WARNING, "client_max_body_size calculation overflow, reverting to default.");
    return;
  }

  size *= multiplier;
  const long long maxSize = 100LL * 1024 * 1024;  // 100 MB in bytes
  if (size > maxSize) {
    Logger::log(WARNING, "client_max_body_size exceeds 100 MB limit, reverting to default.");
    return;
  }
  Logger::log(INFO, "client_max_body_size: " + sizeStr);
  route.setMaxBodySize(size);
  route.setHasMaxBodySize(true);
}

void ConfigurationParser::parseCgiPass(std::string &line, Route &route) {
  std::istringstream iss(line);
  std::string cgiPath;
  iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
  getline(iss, cgiPath);

  if (cgiPath.empty()) {
    Logger::log(WARNING, "cgi_pass is empty, reverting to default.");
    return;
  }

  std::string fullPath = ParsingUtils::getWebservRoot() + cgiPath;
  if (!ParsingUtils::doesPathExist(fullPath))
  {
    Logger::log(WARNING, "cgi_pass path does not exist: " + fullPath + " reverting to default.");
    return;
  }
  if (!ParsingUtils::isRegularFile(fullPath))
  {
    Logger::log(WARNING, "cgi_pass path is not a regular file: " + fullPath + " reverting to default.");
    return;
  }
  if (!ParsingUtils::hasExecutePermissions(fullPath))
  {
    Logger::log(WARNING, "cgi_pass path does not have execute permissions: " + fullPath + " reverting to default.");
    return;
  }
  route.setCGIPath(fullPath);
  route.setHasCGI(true);
  Logger::log(INFO, "CGI path: " + fullPath + " for route " + route.getRoutePath());
}

void ConfigurationParser::checkForDuplicatePorts(const std::map<std::string, Server *> &servers)
{
  std::set<int> usedPorts;
  for (std::map<std::string, Server*>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
    const std::vector<int>& ports = it->second->getPorts();
    for (std::vector<int>::const_iterator portIt = ports.begin(); portIt != ports.end(); ++portIt) {
      int port = *portIt;
      if (usedPorts.find(port) != usedPorts.end()) {
        Logger::log(ERROR, "Duplicate port: " + ParsingUtils::toString(port));
        throw ConfigurationParser::InvalidConfigurationException("Duplicate port: " + ParsingUtils::toString(port));
      }
      usedPorts.insert(port);
    }
  }
}

void ConfigurationParser::checkForDuplicateServerNames(const std::map<std::string, Server*>& servers)
{
  std::set<std::string> usedServerNames;
  for (std::map<std::string, Server*>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
    const std::string& serverName = it->second->getServerName();
    if (usedServerNames.find(serverName) != usedServerNames.end()) {
      Logger::log(ERROR, "Duplicate server name: " + serverName);
      throw ConfigurationParser::InvalidConfigurationException("Duplicate server name: " + serverName);
    }
    usedServerNames.insert(serverName);
  }
}

void ConfigurationParser::checkForDuplicateRoutes(const std::map<std::string, Server*>& servers) {
    for (std::map<std::string, Server*>::const_iterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt) {
        const Server* server = serverIt->second;
        std::set<std::string> uniqueRoutes;

        const std::map<std::string, Route>& routes = server->getRoutes();
        for (std::map<std::string, Route>::const_iterator routeIt = routes.begin(); routeIt != routes.end(); ++routeIt) {
            const std::string& routePath = routeIt->second.getRoutePath();
            if (!uniqueRoutes.insert(routePath).second) {
                // Duplicate route found within the same server
                std::ostringstream errMsg;
                errMsg << "Duplicate route detected in server '" << serverIt->first << "': " << routePath;
                throw::ConfigurationParser::InvalidConfigurationException(errMsg.str());
            }
        }
    }
}
void ConfigurationParser::cleanupServers(std::map<std::string, Server*>& servers) {
  for (std::map<std::string, Server*>::iterator it = servers.begin(); it != servers.end(); ++it) {
    Logger::log(INFO, "DELETING SERVER : " + it->first);
    delete it->second; // Delete the Server object
  }
  servers.clear(); // Clear the map
}

