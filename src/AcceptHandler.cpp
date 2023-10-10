#include "../inc/AcceptHandler.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <cerrno>

AcceptHandler::AcceptHandler(int fd) : listen_fd(fd) {}

void AcceptHandler::handle_event(uint32_t /*events*/) {
	sockaddr_in client_addr = {};
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd == -1) {
		std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
	} else {
		std::cout << "Accepted a connection" << std::endl;
		close(client_fd); // Close the connection immediately for now
	}
}

int AcceptHandler::get_handle() const {
	return listen_fd;
}
