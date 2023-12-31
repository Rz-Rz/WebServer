#include "../inc/Reactor.hpp"
#include <iostream>
#include <sys/epoll.h>
#include <string.h>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include "EventHandler.hpp"
#include "Logger.hpp"
#include "ParsingUtils.hpp"
#include "SystemUtils.hpp"
#include "RequestHandler.hpp"

Reactor::Reactor() {
	epfd = epoll_create(1);
	if (epfd == -1) {
		throw std::runtime_error("Error creating epoll file descriptor: " + std::string(strerror(errno)));
	}
}

Reactor::~Reactor() {
  for (std::map<int, EventHandler*>::iterator it = handlers.begin(); it != handlers.end(); ++it) {
    delete it->second;  // Delete EventHandler objects
  }
  handlers.clear();  // Clear the map
  close(epfd);
}

void Reactor::removeFromInactivityList(int fd) {
  lastActivityMap.erase(fd);
}

void Reactor::registerHandler(EventHandler* eh) {
	int fd = eh->getHandle();

	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Error getting file descriptor flags: " + std::string(strerror(errno)));
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1)
		throw std::runtime_error("Error setting non-blocking mode: " + std::string(strerror(errno)));

	epoll_event event = {};
	event.events = EPOLLIN | EPOLLOUT;
	event.data.ptr = eh;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == -1)
		throw std::runtime_error("Error adding epoll event: " + std::string(strerror(errno)));
	Logger::log(INFO, "Handler registered for fd: " + ParsingUtils::toString(fd));
	handlers[fd] = eh;
	// Add the file descriptor to the lastActivityMap with the current time
	// Check if the EventHandler is a RequestHandler
	if (dynamic_cast<RequestHandler*>(eh) != NULL) {
		// Add the file descriptor to the lastActivityMap with the current time
		lastActivityMap[fd] = time(NULL);
		Logger::log(INFO, "Added fd to lastActivityMap: " + ParsingUtils::toString(fd));
	}
}

void Reactor::deregisterHandler(int fd) {
  if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1)
    throw std::runtime_error("Error deleting epoll event: " + std::string(strerror(errno)));
  handlers.erase(fd);
}

void Reactor::event_loop() {
	time_t lastCheckTime = time(NULL);
	while (true) {
		epoll_event events[2000];
		int nfds = epoll_wait(epfd, events, 2000, -1);
		if (nfds == -1) {
			Logger::log(ERROR, "Error in epoll_wait: " + std::string(strerror(errno)));
			return;
		}
		for (int n = 0; n < nfds; ++n) {
			EventHandler* eh = (EventHandler*)events[n].data.ptr;
			eh->handleEvent(events[n].events);
		}
		time_t currentTime = time(NULL);
		if (currentTime - lastCheckTime >= 5) { // 5 seconds timeout for inactivity check 
			Logger::log(INFO, "Checking for inactive clients");
			removeInactiveClients(5); // Perform the check
			lastCheckTime = currentTime; // Update the last check time
		}
	}
}

void Reactor::updateLastActivity(int fd) {
  lastActivityMap[fd] = time(NULL);
}

void Reactor::removeInactiveClients(int timeout) {
	time_t currentTime = time(NULL);
	for (std::map<int, time_t>::iterator it = lastActivityMap.begin(); it != lastActivityMap.end(); /* no increment here */) {
		if (currentTime - it->second > timeout) {
			Logger::log(INFO, "Removing inactive client: " + ParsingUtils::toString(it->first));
			EventHandler *eh = handlers[it->first];
			int fd = it->first;
			// Erase and advance iterator in a safe way
			// First, deregister the handler from epoll and inactivityMap
			deregisterHandler(fd);
			// Then close the file descriptor
			if (fd >= 0) {
				SystemUtils::closeUtil(fd);
			}
			// Finally, delete the EventHandler object
			if (eh) {
				delete eh;
			}
			lastActivityMap.erase(it++);
		} else {
			++it;
		}
	}
}

bool Reactor::isClientInactive(int fd) {
    time_t lastActivity = getLastActivityTime(fd); // Retrieve last activity time for fd
    time_t currentTime = time(NULL);
    const int timeout = 5; // 5 seconds timeout
    return (currentTime - lastActivity > timeout);
}

time_t Reactor::getLastActivityTime(int fd) {
    std::map<int, time_t>::iterator it = lastActivityMap.find(fd);
    if (it != lastActivityMap.end()) {
        // Found the file descriptor, return the last activity time
        return it->second;
    } else {
        // If the file descriptor is not found, return the start of UNIX EPOCH (1 jan 1970)
        return 0;
    }
}

