#ifndef ACCEPTHANDLER_HPP
#define ACCEPTHANDLER_HPP

#include "EventHandler.hpp"
#include "Reactor.hpp"
#include "Server.hpp"
#include <stdint.h>

class AcceptHandler : public EventHandler {
	private:
		Reactor& reactor;

	public:
		explicit AcceptHandler(int fd, Reactor& reactor);
		~AcceptHandler();
		void handleEvent(uint32_t events);
		void closeConnection(void);
};
#endif
