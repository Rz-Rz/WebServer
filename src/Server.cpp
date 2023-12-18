#include "Server.hpp"
#include <sstream>
#include <iostream>
#include "RouteDebug.hpp"

Server::Server()
{
      this->host = "";
      this->port = 0;
      this->serverName = "";
      this->customErrorPage = false;
      this->maxClientBodySize = 0;
      this->errorPageManager = ErrorPageManager();
      mimeTypes["html"] = "text/html";
      mimeTypes["css"] = "text/css";
      mimeTypes["jpg"] = "image/jpeg";
      mimeTypes["jpeg"] = "image/jpeg";
}

// Setters
void Server::setHost(const std::string& host)
{
	    this->host = host;
}

void Server::setPort(int port)
{
	    this->port = port;
}

void Server::setServerName(const std::string& name)
{
	    this->serverName = name;
}

void Server::setErrorPage(int errorCode, const std::string& pagePath)
{
	    this->errorPages[errorCode] = pagePath;
      this->errorPageManager.setErrorPage(errorCode, pagePath);
}

void Server::hasCustomErrorPage(bool value)
{
	    this->customErrorPage = value;
}

void Server::setMaxClientBodySize(size_t size)
{
	    this->maxClientBodySize = size;
}

void Server::addRoute(const std::string& path, const Route& route)
{
	    this->routes[path] = route;
}

// Getters
std::string Server::getHost() const
{
	    return this->host;
}

int Server::getPort() const
{
	    return this->port;
}

std::string Server::getPortString() const
{
  std::stringstream ss;
  ss << this->port;
  std::string portString = ss.str();
  return portString;
}

std::string Server::getServerName() const
{
	    return this->serverName;
}

std::string Server::getErrorPage(int errorCode) const
{
  std::map<int, std::string>::const_iterator it = this->errorPages.find(errorCode);
  if (it != this->errorPages.end()) {
    return it->second;
  } else {
    return ""; // Return empty string if errorCode is not found
  }
}

bool Server::hasCustomErrorPage(void) const
{
	    return this->customErrorPage;
}

long long Server::getMaxClientBodySize() const
{
	    return this->maxClientBodySize;
}

Route Server::getRoute(const std::string& path) const
{
	    return this->routes.at(path);
}

ErrorPageManager Server::getErrorPageManager() const
{
  return this->errorPageManager;
}

const std::map<std::string, std::string>& Server::getMimeTypes() const
{
  return this->mimeTypes;
}

//debug 
void Server::printRoutes() const
{
  for (std::map<std::string, Route>::const_iterator it = this->routes.begin(); it != this->routes.end(); ++it) {
    std::cout << "Route: " << it->first << std::endl;
    RouteDebug::printRouteInfo(it->second);
  }
}
