#ifndef SESSIONDATA_HPP
#define SESSIONDATA_HPP

#include <string>

class SessionData {
  private:
    std::string sessionId;
    int requestCount;

public:
    SessionData(const std::string& id);
    void incrementRequestCount();

    std::string getSessionId() const;
    void setSessionId(const std::string& id);
    int getRequestCount() const;
    void setRequestCount(int count);

};

#endif
