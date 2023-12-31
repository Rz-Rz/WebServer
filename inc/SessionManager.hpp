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
    std::string createSession(const std::string& sessionId); // overload in cash of cached cookie, but server was restarted

    SessionData* getSessionData(const std::string& sessionId);
    std::string generateUniqueID();
    void debugPrintSessions() const;
    void cleanupSessions();
};

#endif
