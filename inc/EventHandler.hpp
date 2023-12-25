#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP
#include <stdint.h>

class EventHandler { // Event Handler Interface
  // protected:
  //   bool shouldBeDeleted;
	public:
    // EventHandler() : shouldBeDeleted(false) {}
		virtual void handle_event(uint32_t events) = 0; 
		virtual int get_handle() const = 0;
		virtual ~EventHandler() {};

    // void markForDeletion() { shouldBeDeleted = true; }
    // bool isMarkedForDeletion() const { return shouldBeDeleted; }
};
#endif
