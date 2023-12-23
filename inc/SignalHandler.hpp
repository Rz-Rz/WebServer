#include <csignal>
#include <cstdlib>  // For exit
#include <iostream>
#include <map>
#include "Server.hpp"

class SignalHandler {
  public:
    static SignalHandler& getInstance();

    // Register signal handler
    void setupSignalHandlers();

    // Cleanup and shutdown logic
    void cleanup();

    void setServersMap(std::map<std::string, Server*>* map);

private:
    // Private Constructor and Destructor
    SignalHandler() {}
    ~SignalHandler() {}
    std::map<std::string, Server*>* serversMap;

    // Private copy constructor and assignment operator to prevent copying
    SignalHandler(const SignalHandler&);
    SignalHandler& operator=(const SignalHandler&);

    // Static signal handling function
    static void handleSignal(int signal);
};
