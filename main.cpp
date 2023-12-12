#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "Reactor.hpp"
#include "AcceptHandler.hpp"
#include "Logger.hpp"
#include <cerrno>
#include "ConfigurationParser.hpp"
#include "SignalHandling.hpp"


int main(int argc, char** argv) {
  SignalHandler::getInstance().setupSignalHandlers();
  Logger::log(INFO, "Server starting");

    if (argc != 2) {
        Logger::log(ERROR, "Usage error. Correct format: <executable> <config_file>");
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    std::map<std::string, Server> servers;
    Reactor reactor;
    try {
        servers = ConfigurationParser::parse(argv[1]);
        Logger::log(INFO, "Configuration file parsed successfully");
    } catch (const std::exception& e) {
        Logger::log(ERROR, "Configuration error: " + std::string(e.what()));
        std::cerr << "Configuration error: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "Number of servers to process: " << servers.size() << std::endl;
    for (std::map<std::string, Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
        Server& serverConfig = it->second;
        std::cout << "Processing server: " << serverConfig.getServerName() << std::endl;
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            Logger::log(ERROR, "Error creating socket: " + std::string(strerror(errno)));
            std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
            continue; // Proceed to the next configuration
        }
        Logger::log(INFO, "Socket created");

        sockaddr_in serv_addr = {};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(serverConfig.getPort());
        serv_addr.sin_addr.s_addr = INADDR_ANY;

        // Allow socket reuse
        int opt = 1;
        // Set SO_REUSEADDR to allow local address reuse
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
          // Handle setsockopt error
          throw std::runtime_error("Error setting socket options: " + std::string(strerror(errno)));
        }


        if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
            Logger::log(ERROR, "Error binding socket: " + std::string(strerror(errno)));
            std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
            close(server_fd);
            continue; // Proceed to the next configuration
        }
        Logger::log(INFO, "Socket bound");

        if (listen(server_fd, 10) == -1) {
            Logger::log(ERROR, "Error listening on socket: " + std::string(strerror(errno)));
            std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
            close(server_fd);
            continue; // Proceed to the next configuration
        }
        Logger::log(INFO, "Server listening on port " + serverConfig.getPortString());

        // Create and register an AcceptHandler for this server_fd
        AcceptHandler* handler = new AcceptHandler(server_fd, reactor, serverConfig); // Remember to manage memory
        reactor.register_handler(handler);
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
