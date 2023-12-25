#include "SignalHandler.hpp"
#include "Logger.hpp"

SignalHandler& SignalHandler::getInstance() {
  static SignalHandler instance;
  return instance;
}

void SignalHandler::setupSignalHandlers() {
  std::signal(SIGINT, SignalHandler::handleSignal);
}

void SignalHandler::cleanup() {
  std::cout << "Cleaning up resources..." << std::endl;
  if (reactor) {
    Logger::log(INFO, "Cleaning up reactor");
    reactor->~Reactor();
  }
  if (resources.size() > 0) {
    for (std::vector<EventHandler*>::iterator it = resources.begin(); it != resources.end(); ++it) {
      std::cout << "Deleting resource: " << *it << std::endl;
      delete *it; // Free EventHandler instances
    }
    resources.clear();
  }
  if (serversMap) {
    for (std::map<std::string, Server*>::iterator it = serversMap->begin(); it != serversMap->end(); ++it) {
      std::cout << "Deleting server: " << it->first << std::endl;
      delete it->second; // Free Server instances
    }
    serversMap->clear();
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

void SignalHandler::registerResource(EventHandler* resource) {
  resources.push_back(resource);
}

void SignalHandler::deregisterResource(EventHandler* resource) {
  for (std::vector<EventHandler*>::iterator it = resources.begin(); it != resources.end(); ++it) {
    if (*it == resource) {
      resources.erase(it);
      break;
    }
  }
}

void SignalHandler::setReactor(Reactor* reactor) {
  this->reactor = reactor;
}
