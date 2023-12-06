#include <csignal>
#include <iostream>

#include <csignal>
#include <cstdlib>  // For exit
#include <iostream>

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
        // Implement resource cleanup here
    }

private:
    // Private Constructor and Destructor
    SignalHandler() {}
    ~SignalHandler() {}

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
