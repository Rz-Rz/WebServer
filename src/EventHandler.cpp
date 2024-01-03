#include "EventHandler.hpp"
#include "SystemUtils.hpp"

EventHandler::EventHandler(): handle(-1) {}

EventHandler::~EventHandler() {}

void EventHandler::setHandle(int fd) {
    handle = fd;
}

int &EventHandler::getHandle(void) {
    return handle;
}
