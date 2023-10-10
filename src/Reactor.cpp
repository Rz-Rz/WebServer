#include "../inc/Reactor.hpp"
#include <iostream>
#include <sys/epoll.h>
#include <string.h>
#include <cerrno>
#include <unistd.h>



Reactor::Reactor() {
	epfd = epoll_create(1);
	if (epfd == -1) {
		throw std::runtime_error("Error creating epoll file descriptor: " + std::string(strerror(errno)));
	}
}

Reactor::~Reactor() {
	close(epfd);
}

void Reactor::register_handler(EventHandler* eh) {
	int fd = eh->get_handle();
	epoll_event event = {};
	event.events = EPOLLIN;
	event.data.ptr = eh;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1) {
		throw std::runtime_error("Error adding epoll event" + std::string(strerror(errno)));
	}
	handlers[fd] = eh;
}

void Reactor::event_loop() {
	while (true) {
		epoll_event events[10];
		int nfds = epoll_wait(epfd, events, 10, -1);
		if (nfds == -1) {
			std::cerr << "Error in epoll_wait: " << strerror(errno) << std::endl;
			return;
		}
		for (int n = 0; n < nfds; ++n) {
			EventHandler* eh = (EventHandler*)events[n].data.ptr;
			eh->handle_event(events[n].events);
		}
	}
}
