#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include "Route.hpp"  // Assume this is the header file for your Route class
#include "ErrorPageManager.hpp"

class Server {
	public:
		Server();
		void setHost(const std::string& host);
    void setPorts(const std::vector<int>& ports);
		void setServerName(const std::string& name);
		void setErrorPage(int errorCode, const std::string& pagePath);
		void hasCustomErrorPage(bool value);
		void setMaxClientBodySize(size_t size);
		void addRoute(const std::string& path, const Route& route);

		std::string getHost() const;
    const std::vector<int>& getPorts() const;
    std::string getPortsString() const;
		std::string getServerName() const;
		std::string getErrorPage(int errorCode) const;
		bool hasCustomErrorPage(void) const;
		long long getMaxClientBodySize() const;
		Route getRoute(const std::string& path) const;
    std::map<std::string, Route> getRoutes() const;
    ErrorPageManager getErrorPageManager() const;
    const std::map<std::string, std::string>& getMimeTypes() const;

    //debug
    void printRoutes() const;

	private:
    Server(const Server& other);
    Server& operator=(const Server& other);
		std::string host;
    ErrorPageManager errorPageManager;
    std::vector<int> ports;
		std::string serverName;
		std::map<int, std::string> errorPages;
		bool customErrorPage;
		long long maxClientBodySize;
		std::map<std::string, Route> routes;
    std::map<std::string, std::string> mimeTypes;
};

#endif
