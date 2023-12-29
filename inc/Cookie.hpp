#ifndef COOKIE_HPP
#define COOKIE_HPP

#include <string>

class Cookie {
  private:
    std::string cookieName;
    std::string cookieValue;

  public:
    Cookie(const std::string& name, const std::string& value);
    Cookie();
    std::string getCookieName() const;
    std::string getCookieValue() const;
    std::string getCookieString() const;
};

#endif
