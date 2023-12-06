#include "RequestHandler.hpp"
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sys/epoll.h>
#include <errno.h>
#include "Logger.hpp"
#include "ErrorPageManager.hpp"

RequestHandler::RequestHandler(int fd, Server& serverInstance) : client_fd(fd), server(serverInstance) {}

RequestHandler::~RequestHandler() {
  close(client_fd);
}

void RequestHandler::handle_event(uint32_t events) {
  if (events & EPOLLIN) {
    char buffer[1024];
    ssize_t bytes_read = read(client_fd, buffer, 1024);
    if (bytes_read > 0) {
      buffer[bytes_read] = '\0';
      // Append data to the parser
      parser.appendData(std::string(buffer, bytes_read));
      std::cout << "Received " << bytes_read << " bytes from client" << std::endl;

      // Check if the request line and headers are parsed
      if (parser.isRequestLineParsed() && parser.areHeadersParsed()) {
        // Process the request
        // Example: handleRequest();
        RequestHandler::handleRequest(server);
      }
    }
    else if (bytes_read == 0) {
      std::cout << "Client disconnected" << std::endl;
      delete this;
    }
    else {
      std::cerr << "Error reading from client: " << strerror(errno) << std::endl;
      delete this;
    }
  }
}

int RequestHandler::get_handle() const {
  return client_fd;
}

std::string RequestHandler::getFilePathFromUri(const Route& route, const std::string& uri) {
    std::string filePath = route.getRootDirectoryPath();

    // Append the URI to the root directory path
    if (!uri.empty() && uri != "/") {
        filePath += uri;
    } else {
        // Use the default file if the URI is just '/'
        filePath += route.getDefaultFile();
    }
    return filePath;
}

void RequestHandler::sendErrorResponse(int errorCode) {
  std::string errorPageContent = server.getErrorPageManager().getErrorPage(errorCode);
  std::ostringstream responseStream;
  responseStream << "HTTP/1.1 " << errorCode << " " << server.getErrorPageManager().errorCodeMessageParser(errorCode) << "\r\n";
  responseStream << "Content-Type: text/html\r\n";
  responseStream << "Content-Length: " << errorPageContent.size() << "\r\n";
  responseStream << "\r\n";
  responseStream << errorPageContent;

  std::string response = responseStream.str();
  write(client_fd, response.c_str(), response.size());
}

std::string generateDirectoryListingPage(const std::vector<std::string>& contents, const std::string& directoryPath) {
    std::ostringstream html;

    // Basic HTML structure
    html << "<html><head><title>Directory Listing of " << directoryPath << "</title></head><body>";
    html << "<h2>Directory Listing of " << directoryPath << "</h2>";
    html << "<ul>";

    // List each item in the directory
    for (std::vector<std::string>::const_iterator it = contents.begin(); it != contents.end(); ++it) {
        // Assuming directoryPath is formatted correctly (e.g., ends with '/')
        std::string itemPath = directoryPath + *it;

        // Create a list item with a link for each entry
        html << "<li><a href=\"" << itemPath << "\">" << *it << "</a></li>";
    }

    html << "</ul>";
    html << "</body></html>";

    return html.str();
}

void RequestHandler::handleRequest(const Server& server) {
  Route route;
  if (parser.getMethod() == "GET") {
    try {
      route = server.getRoute(parser.getUri());
    } catch (const std::out_of_range& e) {
      // No route found for this URI
      sendErrorResponse(404);
      return;
    }
    if (!route.getGetMethod()) {
      // Method not allowed for this route
      sendErrorResponse(405);
      return;
    }
    if (route.getRedirect()) {
      sendRedirectResponse(route.getRedirectLocation());
      return;
    }
    std::string filePath = getFilePathFromUri(route, parser.getUri());
    if (ParsingUtils::doesPathExistAndReadable(filePath)) {
      std::string fileContent = ParsingUtils::readFile(filePath);
      sendSuccessResponse("200 OK", "text/html", fileContent);
    } else {
      sendErrorResponse(404);
    }
  }
  // Additional handling for other methods...
}

void RequestHandler::sendRedirectResponse(const std::string& redirectLocation) {
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
    write(client_fd, response.c_str(), response.size());

    std::cout << "Redirecting to: " << redirectLocation << std::endl;
}

void RequestHandler::sendSuccessResponse(const std::string& statusCode, const std::string& contentType, const std::string& content) {
    std::ostringstream responseStream;

    // Start building the HTTP response
    responseStream << "HTTP/1.1 " << statusCode << "\r\n";

    // Adding headers
    responseStream << "Content-Type: " << contentType << "\r\n";
    responseStream << "Content-Length: " << content.size() << "\r\n";

    // If you want to keep the connection alive, you can add Connection header
    // responseStream << "Connection: keep-alive\r\n";

    // Header and content separation
    responseStream << "\r\n";

    // Append the actual content
    responseStream << content;

    // Convert the stream to a string
    std::string response = responseStream.str();

    // Send the response to the client
    write(client_fd, response.c_str(), response.size());

    std::cout << "Sent response with status code: " << statusCode << std::endl;
}

