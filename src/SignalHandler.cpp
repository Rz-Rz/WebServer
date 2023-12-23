#include "SignalHandler.hpp"


SignalHandler& SignalHandler::getInstance() {
  static SignalHandler instance;
  return instance;
}

void SignalHandler::setupSignalHandlers() {
  std::signal(SIGINT, SignalHandler::handleSignal);
  // Add other signals if needed
}

void SignalHandler::cleanup() {
  std::cout << "Cleaning up resources..." << std::endl;
  if (serversMap) {
    for (std::map<std::string, Server*>::iterator it = serversMap->begin(); it != serversMap->end(); ++it) {
      std::cout << "Deleting server: " << it->first << std::endl;
      delete it->second; // Free Server instances
    }
    serversMap->clear();
  } else {
    std::cout << "No servers to cleanup" << std::endl;
  }
}

void SignalHandler::setServersMap(std::map<std::string, Server*>* map) {
  std::cout << "Setting serversMap: " << map->size() << std::endl;
  serversMap = map;
  for (std::map<std::string, Server*>::iterator it = serversMap->begin(); it != serversMap->end(); ++it) {
    std::cout << "Server ready for cleanup: " << it->first << std::endl;
  }
  std::cout << "serversMap set with " << serversMap->size() << " servers." << std::endl;
}

void SignalHandler::handleSignal(int signal) {
  std::cout << "Signal " << signal << " received." << std::endl;
  getInstance().cleanup();
  exit(signal);
}
