#include "RequestHandler.hpp"
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sys/epoll.h>
#include <errno.h>
#include "Logger.hpp"

RequestHandler::RequestHandler(int fd) : client_fd(fd) {}

RequestHandler::~RequestHandler() {
  close(client_fd);
}

void RequestHandler::handle_event(uint32_t events) {
  if (events & EPOLLIN) {
    char buffer[1024];
    ssize_t bytes_read = read(client_fd, buffer, 1024);
    if (bytes_read > 0) {
      buffer[bytes_read] = '\0';
      // Append data to the parser
      parser.appendData(std::string(buffer, bytes_read));
      std::cout << "Received " << bytes_read << " bytes from client" << std::endl;

      // Check if the request line and headers are parsed
      if (parser.isRequestLineParsed() && parser.areHeadersParsed()) {
        // Process the request
        // Example: handleRequest();
        Logger::log(INFO, "Request URI: " + parser.getUri());
        Logger::log(INFO, "Request Method: " + parser.getMethod());
        Logger::log(INFO, "HTTP Version: " + parser.getHttpVersion());
        Logger::log(INFO, "Request Body: " + parser.getBody());
        Logger::log(INFO, "Request Headers:");
        parser.printHeaders();
      }
    }
    else if (bytes_read == 0) {
      std::cout << "Client disconnected" << std::endl;
      delete this;
    }
    else {
      std::cerr << "Error reading from client: " << strerror(errno) << std::endl;
      delete this;
    }
  }
}

int RequestHandler::get_handle() const {
  return client_fd;
}
