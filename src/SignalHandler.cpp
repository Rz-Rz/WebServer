#include "SignalHandler.hpp"
#include "Logger.hpp"
#include "ParsingUtils.hpp"

SignalHandler& SignalHandler::getInstance() {
  static SignalHandler instance;
  return instance;
}

void SignalHandler::setupSignalHandlers() {
  std::signal(SIGINT, SignalHandler::handleSignal);
}

void SignalHandler::cleanup() {
  Logger::log(INFO, "Cleaning up resources...");
  if (reactor) {
    Logger::log(INFO, "Cleaning up reactor");
    reactor->~Reactor();
  }
  if (resources.size() > 0) {
    for (std::vector<EventHandler*>::iterator it = resources.begin(); it != resources.end(); ++it) {
      delete *it; // Free EventHandler instances
    }
    resources.clear();
  }
  if (serversMap) {
    for (std::map<std::string, Server*>::iterator it = serversMap->begin(); it != serversMap->end(); ++it) {
      Logger::log(INFO, "Deleting server: " + it->first);
      delete it->second; // Free Server instances
    }
    serversMap->clear();
  }
}

void SignalHandler::setServersMap(std::map<std::string, Server*>* map) {
  Logger::log(INFO, "Setting serversMap: " + ParsingUtils::toString(map->size()) + " servers.");
  serversMap = map;
  for (std::map<std::string, Server*>::iterator it = serversMap->begin(); it != serversMap->end(); ++it) {
    Logger::log(INFO, "Server ready for cleanup: " + it->first);
  }
  Logger::log(INFO, "serversMap set with " + ParsingUtils::toString(serversMap->size()) + " servers.");
}

void SignalHandler::handleSignal(int signal) {
  Logger::log(INFO, "Signal " + ParsingUtils::toString(signal) + " received.");
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
