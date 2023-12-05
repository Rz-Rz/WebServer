#ifndef ACCEPTHANDLER_HPP
#define ACCEPTHANDLER_HPP

#include "EventHandler.hpp"
#include "Reactor.hpp"
#include <stdint.h>

class AcceptHandler : public EventHandler {
	private:
		int listen_fd;
    Reactor& reactor;

	public:
		explicit AcceptHandler(int fd, Reactor& reactor);
		void handle_event(uint32_t events);
		int get_handle() const;
};
#endif
