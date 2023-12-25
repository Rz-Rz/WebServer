#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <map>
#include "EventHandler.hpp"

class Reactor {
	private:
		int epfd;
		std::map<int, EventHandler*> handlers;

	public:
		Reactor();
		~Reactor();
		void register_handler(EventHandler* eh);
    void deregisterHandler(int fd);
		void event_loop();
};

#endif
