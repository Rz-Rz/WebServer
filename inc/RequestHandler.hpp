#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "HTTPRequestParser.hpp"
#include "EventHandler.hpp"

class RequestHandler : public EventHandler {
  private: 
    int client_fd;
    HTTPRequestParser parser;

  public:
    explicit RequestHandler(int fd);
    virtual ~RequestHandler();

    void handle_event(uint32_t events);
    int get_handle() const;
};
#endif
