#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "HTTPRequestParser.hpp"
#include "EventHandler.hpp"
#include "Server.hpp"

class RequestHandler : public EventHandler {
  private: 
    int client_fd;
    HTTPRequestParser parser;
    Server server;

  public:
    explicit RequestHandler(int fd, Server& server);
    virtual ~RequestHandler();

    void handle_event(uint32_t events);
    void handleRequest(const Server& server);
    void sendErrorResponse(int errorCode);
    void sendRedirectResponse(const std::string& redirectLocation);
    void sendSuccessResponse(const std::string& statusCode, const std::string& contentType, const std::string& content);
    std::string getFilePathFromUri(const Route& route, const std::string& uri);
    int get_handle() const;
};
#endif
