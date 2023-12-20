#include <csignal>
#include <cstdlib>  // For exit
#include <iostream>
#include <map>
#include "Server.hpp"

class SignalHandler {
public:
    static SignalHandler& getInstance() {
        static SignalHandler instance;
        return instance;
    }

    // Register signal handler
    void setupSignalHandlers() {
        std::signal(SIGINT, SignalHandler::handleSignal);
        // Add other signals if needed
    }

    // Cleanup and shutdown logic
    void cleanup() {
        std::cout << "Cleaning up resources..." << std::endl;
        if (serversMap) {
            for (std::map<std::string, Server*>::iterator it = serversMap->begin(); it != serversMap->end(); ++it) {
                delete it->second; // Free Server instances
            }
            serversMap->clear();
        }
        // Implement resource cleanup here
    }

    void setServersMap(std::map<std::string, Server*>* map) {
      serversMap = map;
    }

private:
    // Private Constructor and Destructor
    SignalHandler() {}
    ~SignalHandler() {}
    std::map<std::string, Server*>* serversMap;

    // Private copy constructor and assignment operator to prevent copying
    SignalHandler(const SignalHandler&);
    SignalHandler& operator=(const SignalHandler&);

    // Static signal handling function
    static void handleSignal(int signal) {
        std::cout << "Signal " << signal << " received." << std::endl;
        getInstance().cleanup();
        exit(signal);
    }
};
