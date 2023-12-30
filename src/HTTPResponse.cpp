#include "HTTPResponse.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string.h>
#include "Logger.hpp"



void HTTPResponse::sendErrorResponse(int errorCode, const Server* server, int client_fd) {
  std::string errorPageContent;
  std::string errorMessage;
  if (server != NULL)
  {
    errorPageContent = server->getErrorPageManager().getErrorPage(errorCode);
    std::cout << "Error page content: " << errorPageContent << std::endl;
    errorMessage = server->getErrorPageManager().errorCodeMessageParser(errorCode);
  } else {
    // Default error message and content
    std::ostringstream errorContentStream;
    errorContentStream << "<html><body><h1>Error " << errorCode << "</h1></body></html>";
    errorPageContent = errorContentStream.str();
    errorMessage = "Error"; // Generic error message
  }
  std::ostringstream responseStream;
  responseStream << "HTTP/1.1 " << errorCode << " " << errorMessage << "\r\n";  // Use errorMessage
  responseStream << "Content-Type: text/html\r\n";
  responseStream << "Content-Length: " << errorPageContent.size() << "\r\n";
  responseStream << "Connection: close\r\n";
  responseStream << "\r\n";
  responseStream << errorPageContent;

  std::string response = responseStream.str();
  if (write(client_fd, response.c_str(), response.size()) <= 0)
    Logger::log(ERROR, "Error sending error response: " + std::string(strerror(errno)));
  return;
}


void HTTPResponse::sendRedirectResponse(const std::string& redirectLocation, int client_fd) {
    std::ostringstream responseStream;

    // HTTP status code 302 for temporary redirection
    responseStream << "HTTP/1.1 302 Found\r\n";

    // Redirect location header
    responseStream << "Location: " << redirectLocation << "\r\n";

    // Additional headers can be added here if needed
    responseStream << "Content-Length: 0\r\n";
    responseStream << "Connection: close\r\n";

    // End of headers
    responseStream << "\r\n";

    std::string response = responseStream.str();
    if (write(client_fd, response.c_str(), response.size()) <= 0)
      Logger::log(ERROR, "Error sending redirect response: " + std::string(strerror(errno)));
    else
      Logger::log(INFO, "Sent redirect response to: " + redirectLocation);
}

void HTTPResponse::sendSuccessResponse(const std::string& statusCode, const std::string& contentType, const std::string& content, Cookie cookie, int client_fd) {
	std::ostringstream responseStream;

	// Start building the HTTP response
	responseStream << "HTTP/1.1 " << statusCode << "\r\n";

	// Adding headers
	responseStream << "Content-Type: " << contentType << "\r\n";
	responseStream << "Content-Length: " << content.size() << "\r\n";
	// Check if a cookie needs to be set
	if (!cookie.getCookieName().empty()) {
		Logger::log(INFO, "Setting cookie: " + cookie.getCookieString());
		responseStream << "Set-Cookie: " << cookie.getCookieString() << "\r\n";
	}

	// Inform the client that the connection will be closed after the response
	responseStream << "Connection: close\r\n";

	// If you want to keep the connection alive, you can add Connection header
	// responseStream << "Connection: keep-alive\r\n";
	// Header and content separation
	responseStream << "\r\n";

	// Append the actual content
	responseStream << content;

	// Convert the stream to a string
	std::string response = responseStream.str();

	// Send the response to the client
	if (write(client_fd, response.c_str(), response.size()) <= 0)
		Logger::log(ERROR, "Error sending response: " + std::string(strerror(errno)));
	else
		Logger::log(INFO, "Sent response with status code: " + statusCode);
}

std::string HTTPResponse::modifyHtmlContentForSession(const std::string& htmlContent, const SessionData& sessionData) {
    std::string modifiedContent = htmlContent;
    // Find a placeholder in your HTML where you want to insert session info
    size_t placeholderPos = modifiedContent.find("[SESSION_INFO]");
    if (placeholderPos != std::string::npos) {
        std::stringstream sessionInfo;
        sessionInfo << "Session ID: " << sessionData.getSessionId() << "<br>"
                    << "Request count: " << sessionData.getRequestCount();
        modifiedContent.replace(placeholderPos, std::string("[SESSION_INFO]").length(), sessionInfo.str());
    }
    return modifiedContent;
}

std::string HTTPResponse::setCookie(const std::string& cookieName, const std::string& cookieValue) {
    // Format the Set-Cookie header string
    return "Set-Cookie: " + cookieName + "=" + cookieValue + "; Path=/; HttpOnly";
}
