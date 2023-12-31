#include "ServerManager.hpp"

ServerManager& ServerManager::getInstance() {
  static ServerManager instance;
  return instance;
}

SessionManager& ServerManager::getSessionManager() {
  return sessionManager;
}

void ServerManager::setServersMap(std::map<std::string, Server*>* map) {
  serversMap = map;
}

std::map<std::string, Server*>* ServerManager::getServersMap() const {
  return serversMap;
}

ServerManager::ServerManager() {}

ServerManager::~ServerManager() {}
