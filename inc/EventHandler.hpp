#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP
#include <stdint.h>

class EventHandler {
	public:
		virtual void handle_event(uint32_t events) = 0; 
		virtual int get_handle() const = 0;
    virtual void closeConnection(void) = 0;
		virtual ~EventHandler() {};

};
#endif
