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
#include "Logger.hpp"
#include "SystemUtils.hpp"

AcceptHandler::AcceptHandler(int fd, Reactor &reactor) : reactor(reactor) {
  EventHandler::setHandle(fd);
}

AcceptHandler::~AcceptHandler() {}

void AcceptHandler::handleEvent(uint32_t /*events*/) {
	Logger::log(INFO, "Accepting a connection");
	sockaddr_in client_addr = {};
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(EventHandler::getHandle(), (struct sockaddr*)&client_addr, &client_len);
	if (client_fd == -1) {
		Logger::log(ERROR, "Error accepting connection: " + std::string(strerror(errno)));
	}
	else 
	{
		Logger::log(INFO, "Registering handler for connection");
		// Create and register a RequestHandler for this client_fd
		EventHandler* handler = new RequestHandler(client_fd, &reactor);
		reactor.registerHandler(handler);
	}
}

void AcceptHandler::closeConnection(void) {
  if (EventHandler::getHandle() != -1)
    SystemUtils::closeUtil(EventHandler::getHandle());
}
