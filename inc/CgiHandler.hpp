#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <sstream>
#include <sys/epoll.h>
#include "EventHandler.hpp"
#include "Reactor.hpp"

class CgiHandler : public EventHandler {
private:
    int client_fd;
    std::ostringstream cgiOutputBuffer; // Buffer to store CGI output
    Reactor *reactor;
    int childPid;

public:
    CgiHandler(const std::string& filePath, const std::string& queryString, int client_fd, Reactor* reactor);
    ~CgiHandler();
    void setCGIEnvironment(const std::string& queryString);
    int executeCGI(const std::string& filePath);
    void handleEvent(uint32_t events);
    void closeConnection(void);
};


#endif
