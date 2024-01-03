#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP
#include <stdint.h>

class EventHandler {
  protected:
    int handle;

	public:
    EventHandler();
		virtual void handleEvent(uint32_t events) = 0; 
    virtual void closeConnection(void) = 0;
		virtual ~EventHandler();
    virtual void setHandle(int fd);
    virtual int &getHandle(void);

};
#endif
