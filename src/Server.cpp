#include "Server.hpp"

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

std::string Server::getServerName() const
{
	    return this->serverName;
}

std::string Server::getErrorPage(int errorCode) const
{
	    return this->errorPages.at(errorCode);
}

bool Server::hasCustomErrorPage(void) const
{
	    return this->customErrorPage;
}

size_t Server::getMaxClientBodySize() const
{
	    return this->maxClientBodySize;
}

Route Server::getRoute(const std::string& path) const
{
	    return this->routes.at(path);
}
