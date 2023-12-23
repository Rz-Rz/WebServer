#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "Reactor.hpp"
#include "AcceptHandler.hpp"
#include "Logger.hpp"
#include <cerrno>
#include "ConfigurationParser.hpp"
#include "SignalHandler.hpp"
#include "ServerManager.hpp"


int main(int argc, char** argv) {
  SignalHandler::getInstance().setupSignalHandlers();
  Logger::log(INFO, "Server starting");

  if (argc != 2) {
    Logger::log(ERROR, "Usage error. Correct format: <executable> <config_file>");
    std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
    return 1;
  }

  std::map<std::string, Server*> servers;
  Reactor reactor;
  try {
    servers = ConfigurationParser::parse(argv[1]);
    ConfigurationParser::checkValidity(servers);
    Logger::log(INFO, "Configuration file parsed successfully");
  } catch (const std::exception& e) {
    ConfigurationParser::cleanupServers(servers);
    Logger::log(ERROR, "Configuration error: " + std::string(e.what()));
    return 1;
  }
  std::cout << "Number of servers to process: " << servers.size() << std::endl;
  SignalHandler::getInstance().setServersMap(&servers);
  for (std::map<std::string, Server*>::iterator it = servers.begin(); it != servers.end(); ++it) {
      Server* serverConfig = it->second;
      std::cout << "Processing server: " << serverConfig->getServerName() << " with routes:" << std::endl;
      serverConfig->printRoutes();

      // Iterate over each port in the server configuration
      const std::vector<int>& ports = serverConfig->getPorts();
      for (std::vector<int>::const_iterator portIt = ports.begin(); portIt != ports.end(); ++portIt) {
        int port = *portIt;
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
          Logger::log(ERROR, "Error creating socket: " + std::string(strerror(errno)));
          std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
          continue; // Proceed to the next port
        }

        std::ostringstream portStr;
        portStr << port;
        Logger::log(INFO, "Socket created for port " + portStr.str());

        sockaddr_in serv_addr = {};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        // Check if a specific host IP is configured
        if (!serverConfig->getHost().empty())
          // Convert and set the specified IP address
          serv_addr.sin_addr.s_addr = inet_addr(serverConfig->getHost().c_str());
        else
          serv_addr.sin_addr.s_addr = INADDR_ANY;

        // Allow socket reuse
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
          Logger::log(ERROR, "Error setting socket options: " + std::string(strerror(errno)));
          std::cerr << "Error setting socket options: " << strerror(errno) << std::endl;
          close(server_fd);
          continue; // Proceed to the next port
        }

        if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
          Logger::log(ERROR, "Error binding socket: " + std::string(strerror(errno)));
          std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
          close(server_fd);
          continue; // Proceed to the next port
        }
        Logger::log(INFO, "Socket bound to port " + portStr.str());

        if (listen(server_fd, 10) == -1) {
          Logger::log(ERROR, "Error listening on socket: " + std::string(strerror(errno)));
          std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
          close(server_fd);
          continue; // Proceed to the next port
        }
        Logger::log(INFO, "Server listening on port " + portStr.str());

        // Create and register an AcceptHandler for this server_fd
        AcceptHandler* handler = new AcceptHandler(server_fd, reactor, serverConfig); // Manage memory carefully
        SignalHandler::getInstance().registerResource(handler);
        reactor.register_handler(handler);
      }
    }

    try {
        reactor.event_loop();
        Logger::log(INFO, "Reactor event loop started");
    } catch (const std::exception& e) {
        Logger::log(ERROR, "Reactor error: " + std::string(e.what()));
        std::cerr << "Reactor error: " << e.what() << std::endl;
        // Clean-up code here
        return 1;
    }

    Logger::log(INFO, "Server shutting down");
    // Clean-up code here (e.g., closing file descriptors)
    return 0;
}
