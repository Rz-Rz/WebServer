#include "Cookie.hpp"

Cookie::Cookie() {}

Cookie::Cookie(const std::string& name, const std::string& value) : cookieName(name), cookieValue(value) {}

std::string Cookie::getCookieName() const { return cookieName; }

std::string Cookie::getCookieValue() const { return cookieValue; }

std::string Cookie::getCookieString() const { return cookieName + "=" + cookieValue; }
