#include "SessionManager.hpp"
#include <cstdlib>
#include <ctime>
#include <stdexcept>

std::string SessionManager::generateUniqueID() {
  // Generate a random unique ID for the session
  std::string id;
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  srand((unsigned) time(NULL));

  for (int i = 0; i < 10; ++i) { // 10 character long ID
    id += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return id;
}

std::string SessionManager::createSession() {
  std::string sessionId;
  std::pair<std::map<std::string, SessionData>::iterator, bool> result;
  do {
    sessionId = generateUniqueID();
    result = sessions.insert(std::pair<std::string, SessionData>(sessionId, SessionData(sessionId)));
  } while (!result.second); // Continue looping until the session ID is unique and insertion is successful

  return sessionId;
}

void SessionManager::cleanupSessions() {
  for (std::map<std::string, SessionData>::iterator it = sessions.begin(); it != sessions.end(); ++it) {
    if (it->second.getRequestCount() == 0) {
      sessions.erase(it);
    }
  }
}

SessionData& SessionManager::getSessionData(const std::string& sessionId) {
    std::map<std::string, SessionData>::iterator it = sessions.find(sessionId);
    if (it == sessions.end()) {
        // Session ID not found in the map
        throw std::runtime_error("Session ID not found: " + sessionId);
    }
    return it->second;
}

SessionManager::~SessionManager() {
  cleanupSessions();
}
