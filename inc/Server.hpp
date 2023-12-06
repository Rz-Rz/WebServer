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
		void setPort(int port);
		void setServerName(const std::string& name);
		void setErrorPage(int errorCode, const std::string& pagePath);
		void hasCustomErrorPage(bool value);
		void setMaxClientBodySize(size_t size);
		void addRoute(const std::string& path, const Route& route);

		std::string getHost() const;
		int getPort() const;
    std::string getPortString() const;
		std::string getServerName() const;
		std::string getErrorPage(int errorCode) const;
		bool hasCustomErrorPage(void) const;
		long long getMaxClientBodySize() const;
		Route getRoute(const std::string& path) const;
    ErrorPageManager getErrorPageManager() const;

	private:
		std::string host;
    ErrorPageManager errorPageManager;
		int port;
		std::string serverName;
		std::map<int, std::string> errorPages;
		bool customErrorPage;
		long long maxClientBodySize;
		std::map<std::string, Route> routes;
};

#endif
