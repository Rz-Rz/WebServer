#include "AcceptHandler.hpp"
#include "RequestHandler.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <cerrno>
#include "Reactor.hpp"

AcceptHandler::AcceptHandler(int fd, Reactor &reactor) : listen_fd(fd), reactor(reactor) {}

void AcceptHandler::handle_event(uint32_t /*events*/) {
  std::cout << "AcceptHandler::handle_event" << std::endl;
	sockaddr_in client_addr = {};
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
  if (client_fd == -1) {
    std::cerr << "Error accepting client: " << strerror(errno) << std::endl;
  }
  else 
  {
    std::cout << "Accepting a connection" << std::endl;
    // Create and register a RequestHandler for this client_fd
    EventHandler* handler = new RequestHandler(client_fd); // Remember to manage memory
    reactor.register_handler(handler);
  }
}

int AcceptHandler::get_handle() const {
	return listen_fd;
}
