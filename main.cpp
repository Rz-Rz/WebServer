#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "Reactor.hpp"
#include "AcceptHandler.hpp"
#include <cerrno>
#include "ConfigurationParser.hpp"



int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
    return 1;
  }
  else {
    std::map<std::string, Server> servers;
    servers = ConfigurationParser::parse(argv[1]);
  }
	// int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	// if (listen_fd == -1) {
	// 	std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
	// 	return 1;
	// }
	//
	// sockaddr_in serv_addr = {};
	// serv_addr.sin_family = AF_INET;
	// serv_addr.sin_port = htons(8080);
	// serv_addr.sin_addr.s_addr = INADDR_ANY;
	//
	// if (bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
	// 	std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
	// 	close(listen_fd);
	// 	return 1;
	// }
	//
	// if (listen(listen_fd, 10) == -1) {
	// 	std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
	// 	close(listen_fd);
	// 	return 1;
	// }
	//
	// try {
	// 	Reactor reactor;
	// 	AcceptHandler acceptHandler(listen_fd);
	// 	reactor.register_handler(&acceptHandler);
	// 	reactor.event_loop();
	// } catch (const std::exception& e) {
	// 	std::cerr << "Error: " << e.what() << std::endl;
	// 	close(listen_fd);
	// 	return 1;
	// }
	//
	// close(listen_fd);
	return 0;

}
