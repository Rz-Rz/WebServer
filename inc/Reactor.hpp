#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <map>
#include <ctime>
#include "EventHandler.hpp"

class Reactor {
	private:
		int epfd;
		std::map<int, EventHandler*> handlers;
    std::map<int, time_t> lastActivityMap;

	public:
		Reactor();
		~Reactor();
		void register_handler(EventHandler* eh);
    void deregisterHandler(int fd);
		void event_loop();
    void updateLastActivity(int fd);
    void removeInactiveClients(int timeout);
    bool isClientInactive(int fd);
    time_t getLastActivityTime(int fd);

};

#endif
