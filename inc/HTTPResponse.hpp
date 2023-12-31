#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include "Server.hpp"
#include "SessionData.hpp"
#include "Cookie.hpp"

class HTTPResponse {
  public:
    static void sendErrorResponse(int errorCode, const Server* server, int client_fd);
    static void sendRedirectResponse(const std::string& redirectLocation, int client_fd);
    static void sendSuccessResponse(const std::string& statusCode, const std::string& contentType, const std::string& content, Cookie cookie, int client_fd);
    static std::string modifyHtmlContentForSession(const std::string& content, const SessionData* sessionData);
    static std::string setCookie(const std::string& cookieName, const std::string& cookieValue);
};

#endif
