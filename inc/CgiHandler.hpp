#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <sstream>
#include <sys/epoll.h>
#include "EventHandler.hpp"

class CgiHandler : public EventHandler {
private:
    int client_fd;
    std::ostringstream cgiOutputBuffer; // Buffer to store CGI output

public:
    CgiHandler(const std::string& filePath, const std::string& queryString, int client_fd);
    ~CgiHandler();
    void setCGIEnvironment(const std::string& queryString);
    int executeCGI(const std::string& filePath);
    void handleEvent(uint32_t events);
    void closeConnection(void);
};


#endif
