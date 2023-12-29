#ifndef SESSIONMANAGER_HPP
#define SESSIONMANAGER_HPP

#include <string>
#include <map>
#include "SessionData.hpp"

class SessionManager {
  private:
    std::map<std::string, SessionData> sessions;

  public:
    ~SessionManager();
    std::string createSession();
    SessionData& getSessionData(const std::string& sessionId);
    std::string generateUniqueID();
    void cleanupSessions();
};

#endif
