#include "SessionManager.hpp"
#include "Logger.hpp"
#include "ParsingUtils.hpp"
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <iostream>

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
  Logger::log(INFO, "Inserted session id: ----" + sessionId + "----");
  return sessionId;
}

std::string SessionManager::createSession(const std::string& sessionId) {
    std::pair<std::map<std::string, SessionData>::iterator, bool> result;
    result = sessions.insert(std::make_pair(sessionId, SessionData(sessionId)));

    if (!result.second) {
        // Handle the case where a session with the provided ID already exists
        Logger::log(ERROR, "Session with ID " + sessionId + " already exists. Not inserting a new one.");
        return ""; // Or handle it some other way
    }

    Logger::log(INFO, "Inserted session id: ----" + sessionId + "----");
    return sessionId;
}


void SessionManager::cleanupSessions() {
  for (std::map<std::string, SessionData>::iterator it = sessions.begin(); it != sessions.end(); ) {
    if (it->second.getRequestCount() == 0) {
      sessions.erase(it++);
    }
    else {
      ++it;
    }
  }
}

SessionData* SessionManager::getSessionData(const std::string& sessionId) {
    std::map<std::string, SessionData>::iterator it = sessions.find(sessionId);
    if (it == sessions.end()) {
      Logger:: log(ERROR, "Session id: ----" + sessionId + "---- not found.");
        return NULL; // Return nullptr if session ID not found
    }
    return &it->second; // Return pointer to the found session data
}

SessionManager::~SessionManager() {
  cleanupSessions();
}

void SessionManager::debugPrintSessions() const {
  Logger::log(INFO, "Current Sessions:");
  for (std::map<std::string, SessionData>::const_iterator it = sessions.begin(); it != sessions.end(); ++it) {
    Logger:: log(INFO, "Session ID: " + it->first + ", Request Count: " + ParsingUtils::toString(it->second.getRequestCount()));
  }
}
