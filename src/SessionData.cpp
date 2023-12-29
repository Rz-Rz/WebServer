#include "SessionData.hpp"

SessionData::SessionData(const std::string& id) : sessionId(id), requestCount(0) {}
void SessionData::incrementRequestCount() { requestCount++; }
std::string SessionData::getSessionId() const { return sessionId; }
void SessionData::setSessionId(const std::string& id) { sessionId = id; }
int SessionData::getRequestCount() const { return requestCount; }
void SessionData::setRequestCount(int count) { requestCount = count; }
