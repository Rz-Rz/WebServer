#include <csignal>
#include <cstdlib>  // For exit
#include <iostream>
#include <map>
#include "Server.hpp"
#include "EventHandler.hpp"
#include "Reactor.hpp"

class SignalHandler {
  public:
    static SignalHandler& getInstance();

    // Register signal handler
    void setupSignalHandlers();

    // Cleanup and shutdown logic
    void cleanup();

    void setServersMap(std::map<std::string, Server*>* map);
    void setReactor(Reactor* reactor);

    void registerResource(EventHandler* resource);
    void deregisterResource(EventHandler* resource);

private:
    // Private Constructor and Destructor
    SignalHandler() {}
    ~SignalHandler() {}
    std::map<std::string, Server*>* serversMap;
    Reactor* reactor;

    // Private copy constructor and assignment operator to prevent copying
    SignalHandler(const SignalHandler&);
    SignalHandler& operator=(const SignalHandler&);

    // Static signal handling function
    static void handleSignal(int signal);

    std::vector<EventHandler*> resources;
};
