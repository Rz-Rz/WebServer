#include "RequestHandler.hpp"
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sys/epoll.h>
#include <errno.h>
#include "HTTPRequestParser.hpp"
#include "Logger.hpp"
#include "ErrorPageManager.hpp"
#include "ParsingUtils.hpp"
#include <cstdlib>
#include "MultipartFormDataParser.hpp"
#include "SystemUtils.hpp"
#include <sys/wait.h>

RequestHandler::RequestHandler(int fd, Server& serverInstance) : client_fd(fd), server(serverInstance), shouldBeDeleted(false) {}

RequestHandler::~RequestHandler() {
  SystemUtils::closeUtil(client_fd);
}

void RequestHandler::handle_event(uint32_t events) {
  if (events & EPOLLIN) {
    char buffer[1024];

    while (true) {
      ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
      if (bytes_read > 0) {
        try {
          parser.appendData(std::string(buffer, bytes_read));
          std::cout << "PACKET RECV ----" << std::endl << std::string(buffer, bytes_read) << std::cout << "PACKET END ----" << std::endl;
        } catch (const HTTPRequestParser::InvalidHTTPVersionException& e) {
          Logger::log(ERROR, std::string("Error Parsing HTTP Request: ") + e.what());
          sendErrorResponse(505);
          closeConnection();
          break;
        }
        catch (const HTTPRequestParser::InvalidMethodException& e) {
          Logger::log(ERROR, "Error Parsing HTTP Request: " + std::string(e.what()));
          sendErrorResponse(405);
          closeConnection();
          break;
        }
        // Check if the entire request has been received
        if (parser.isCompleteRequest()) {
          std::cout << "Received complete request" << std::endl;
          // Process the request
          RequestHandler::handleRequest(server);
          closeConnection();
          break;
        }
      }
      else if (bytes_read == 0) {
        // Client disconnected
        std::cout << "Client disconnected" << std::endl;
        closeConnection();
        break;
      }
      else {
        // Check the error code
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          // Resource temporarily unavailable, which is normal for non-blocking sockets
          std::cout << "No more data available at the moment, waiting for more data" << std::endl;
          break;
        } else {
          // Actual error reading from client
          std::cerr << "Error reading from client: " << strerror(errno) << std::endl;
          closeConnection();
          break;
        }
      }
    }
  }
}

int RequestHandler::get_handle() const {
  return client_fd;
}

std::string RequestHandler::getFilePathFromUri(const Route& route, const std::string& uri) {
    std::string rootPath = route.getRootDirectoryPath();
    std::string filePath = rootPath;

    // Check if the root path ends with a slash and adjust accordingly
    if (!rootPath.empty() && rootPath[rootPath.length() - 1] != '/') {
        filePath += "/";
    }
    // Append the URI to the root directory path, omitting the leading slash in the URI
    if (!uri.empty() && uri != "/") {
        if (uri[0] == '/') {
            filePath += uri.substr(1);
        } else {
            filePath += uri;
        }
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
  // Inform the client that the connection will be closed after the response
  responseStream << "Connection: close\r\n";
  responseStream << "\r\n";
  responseStream << errorPageContent;

  std::string response = responseStream.str();
  write(client_fd, response.c_str(), response.size());
}

std::string RequestHandler::generateDirectoryListingPage(const std::vector<std::string>& contents, const std::string& directoryPath) {
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

void RequestHandler::handleRedirect(const Route& route) {
  sendRedirectResponse(route.getRedirectLocation());
  Logger::log(INFO, "Redirecting to: " + route.getRedirectLocation());
}

void RequestHandler::handleDirectoryRequest(const Route& route)
{
      std::string directoryPath = getFilePathFromUri(route, parser.getUri());
      if (!ParsingUtils::doesPathExist(directoryPath)) {
        Logger::log(ERROR, "Directory does not exist: " + directoryPath);
        sendErrorResponse(403);
        return;
      }
      if (!ParsingUtils::hasReadPermissions(directoryPath)) {
        Logger::log(ERROR, "Directory is not readable: " + directoryPath);
        sendErrorResponse(403);
        return;
      }
      // Directory exists and is readable
      Logger::log(INFO, "Directory listing on GET request: " + directoryPath);
      std::vector<std::string> contents;
      try {
        contents = ParsingUtils::getDirectoryContents(directoryPath);
      } catch (const std::exception& e) {
        Logger::log(ERROR, "500 - Error reading directory contents: " + std::string(e.what()));
        sendErrorResponse(500);
        return;
      }
      std::string directoryListingPage = generateDirectoryListingPage(contents, parser.getUri());
      sendSuccessResponse("200 OK", "text/html", directoryListingPage);
      return;
}

void RequestHandler::handleFileRequest(const Route& route) {
  std::string filePath = getFilePathFromUri(route, parser.getUri());
  if (ParsingUtils::isDirectory(filePath))
  {
    sendErrorResponse(403);
    Logger::log(ERROR, "403 - Directory listing is not enabled: " + filePath);
    return;
  }
  if (ParsingUtils::doesPathExistAndReadable(filePath)) {
    std::string fileContent = ParsingUtils::readFile(filePath);
    sendSuccessResponse("200 OK", "text/html", fileContent);
    return;
  } else {
    sendErrorResponse(404);
    Logger::log(ERROR, "404 - File not found: " + filePath);
    return;
  }
}

std::string RequestHandler::extractRouteFromUri(const std::string& uri) {
  size_t pos = uri.find('?');
  if (pos != std::string::npos) {
    return uri.substr(0, pos);
  }
  return uri;
}

std::string RequestHandler::endWithSlash(const std::string& uri) {
  if (uri[uri.length() - 1] != '/') {
    return uri + "/";
  }
  return uri;
}

void RequestHandler::handleGetRequest(const Server& server) {
  std::string filePath = extractDirectoryPath(parser.getUri());
  Route route;
  try {
    route = server.getRoute(filePath);
  } catch (const std::out_of_range& e) {
    // No route found for this URI
    sendErrorResponse(404);
    Logger::log(ERROR, "404 - No route found for URI: " + parser.getUri());
    return;
  }
  if (!route.getGetMethod()) {
    // Method not allowed for this route
    sendErrorResponse(405);
    Logger::log(ERROR, "405 - Method not allowed for URI: " + parser.getUri());
    return;
  }
  if (route.getRedirect()) {
    handleRedirect(route);
    return;
  }
  else if (route.getDirectoryListing() && !route.getHasDefaultFile()) {
    handleDirectoryRequest(route);
    return;
  }
  else if (route.getHasCGI()) {
    handleCGIRequest(route);
    return;
  }
  else {
    handleFileRequest(route);
    return;
  }
}

void RequestHandler::setCGIEnvironment(const std::string& queryString) {
  setenv("QUERY_STRING", queryString.c_str(), 1);
}

std::string RequestHandler::removeQueryString(const std::string& uri) {
  size_t pos = uri.find('?');
  if (pos != std::string::npos) {
    return uri.substr(0, pos);
  }
  return uri;
}

void RequestHandler::handleCGIRequest(const Route& route) {
  std::string queryString = extractQueryString(parser.getUri());
  std::string filePath = getFilePathFromUri(route, parser.getUri());
  filePath = removeQueryString(filePath);

  if (!ParsingUtils::doesPathExist(filePath)) {
    Logger::log(ERROR, "404 - File not found: " + filePath);
    sendErrorResponse(404);
    return;
  }

  if (!ParsingUtils::hasReadPermissions(filePath)) {
    Logger::log(ERROR, "403 - File is not readable: " + filePath);
    sendErrorResponse(403);
    return;
  }

  if (!ParsingUtils::hasExecutePermissions(filePath)) {
    Logger::log(ERROR, "403 - File is not executable: " + filePath);
    sendErrorResponse(403);
    return;
  }
  setCGIEnvironment(queryString);
  // File exists and is readable and executable
  Logger::log(INFO, "CGI request on GET request: " + filePath);
  std::string output = RequestHandler::executeCGI(filePath);
  sendSuccessResponse("200 OK", "text/html", output);
  return;
}

std::string RequestHandler::executeCGI(const std::string& filePath) {
    int pipefd[2];
    pid_t pid;
    char buf;

    // Create a pipe for the child process's output
    if (pipe(pipefd) == -1) {
        throw std::runtime_error("Failed to create pipe");
    }

    // Fork the current process
    pid = fork();
    if (pid == -1) {
        throw std::runtime_error("Failed to fork process");
    }

    if (pid == 0) {
        // Child process
        // Close the read-end of the pipe, we're not going to read from it
        SystemUtils::closeUtil(pipefd[0]);          
        // Redirect stdout to the write-end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        SystemUtils::closeUtil(pipefd[1]);
        // Convert filePath to a format suitable for execve
        char* execArgs[2];
        execArgs[0] = const_cast<char*>(filePath.c_str());
        execArgs[1] = NULL;  // execve expects a NULL terminated array
        // Execute the CGI script
        execve(execArgs[0], execArgs, environ);
        // Use _exit to terminate the child process if execve fails
        _exit(EXIT_FAILURE);
    } else {
        // Parent process
        // Close the write-end of the pipe, we're not going to write to it
        SystemUtils::closeUtil(pipefd[1]);          
        // Read the output of the CGI script from the read-end of the pipe
        std::ostringstream stream;
        while (read(pipefd[0], &buf, 1) > 0) {
            stream << buf;
        }
        // Close the read-end of the pipe
        SystemUtils::closeUtil(pipefd[0]);          
        // Wait for the child process to finish
        waitpid(pid, NULL, 0);    
        return stream.str();
    }
}

std::string RequestHandler::extractQueryString(const std::string& uri) {
    size_t queryStringPos = uri.find('?');
    if (queryStringPos != std::string::npos) {
        return uri.substr(queryStringPos + 1);
    }
    return "";
}

bool RequestHandler::isPayloadTooLarge(void) {
    std::string contentLengthHeader = parser.getHeader("Content-Length");
    long long contentLength = 0;
    if (!contentLengthHeader.empty()) {
        contentLength = std::atoll(contentLengthHeader.c_str());
        if (contentLength > server.getMaxClientBodySize()) {
            sendErrorResponse(413);
            Logger::log(ERROR, "413 - Payload too large: " + contentLengthHeader);
            return true; // Payload is too large
        }
    }
    return false; // Payload is within the acceptable size
}

std::string RequestHandler::extractFilename(const HTTPRequestParser& parser) {
    std::string uri = parser.getUri();
    size_t lastSlashPos = uri.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        return uri.substr(lastSlashPos + 1);
    }
    return "";
}

std::string RequestHandler::getUploadDirectoryFromUri(const Route& route, const std::string& uri) {
  std::string uploadDir;
  if (route.getAllowFileUpload())
    uploadDir = route.getUploadLocation();
  else
    uploadDir = getFilePathFromUri(route, uri);
  return uploadDir;
}

void RequestHandler::handleFileUpload(const Route& route) {
  std::string filePath = getUploadDirectoryFromUri(route, parser.getUri());
  if (isPayloadTooLarge()) {
    return;
  }

  std::string body = parser.getBody();
  std::cout << "BODY in HANDLE FILE UPLOAD: " << body << std::endl  << "END OF BODY IN HANDLE FILE UPLOAD"  << std::endl;
  std::string boundary = parser.getBoundary();

  if (boundary.empty()) {
    Logger::log(ERROR, "400 - No boundary found in multipart form data");
    sendErrorResponse(400); // Bad Request
    return;
  }

  MultipartFormDataParser multipartParser(body, boundary);
  try {
    multipartParser.parse();
  } catch (const MultipartFormDataParser::MultipartFormDataParserException& e) {
    Logger::log(ERROR, std::string("Error parsing multipart form data: ") + e.what());
    sendErrorResponse(500); // Internal Server Error
    return;
  }

  // Extracting the filename and file content
  const std::map<std::string, std::string>& fileFields = multipartParser.getFileFields();
  if (fileFields.empty()) {
    Logger::log(ERROR, "No files found in the request");
    sendErrorResponse(400); // Bad Request
    return;
  }

  if (!ParsingUtils::doesPathExist(filePath)) {
    Logger::log(ERROR, "Directory does not exist: " + filePath);
    sendErrorResponse(403);
    return;
  }

  if (!ParsingUtils::hasWritePermissions(filePath)) {
    Logger::log(ERROR, "Directory is not writable: " + filePath);
    sendErrorResponse(403);
    return;
  }

  filePath += getFilename(multipartParser);

  // Directory exists and is writable
  Logger::log(INFO, "File upload on POST request: " + filePath);
  std::string fileContent = parser.getBody();
  std::ofstream fileStream(filePath.c_str(), std::ios::out | std::ios::binary);
  if (!fileStream) {
    Logger::log(ERROR, "500 - Error opening file for writing: " + filePath);
    sendErrorResponse(500);
    return;
  }
  fileStream << fileContent;
  fileStream.close();
  sendSuccessResponse("200 OK", "text/html", "File uploaded successfully");
  return;
}

void RequestHandler::handlePostRequest(const Server& server) {
  Route route;
  
  try {
    route = server.getRoute(parser.getUri());
  } catch (const std::out_of_range& e) {
    sendErrorResponse(404);
    Logger::log(ERROR, "404 - No route found for URI: " + parser.getUri());
    return;
  }

  if (!route.getPostMethod()) {
    sendErrorResponse(405);
    Logger::log(ERROR, "405 - Method not allowed for URI: " + parser.getUri());
    return;
  }

  if (route.getAllowFileUpload()) {
    handleFileUpload(route);
  }
}

std::string RequestHandler::extractDirectoryPath(const std::string& filePath) {
    if (filePath.empty()) {
        return "";
    }

    // Remove query string if present
    size_t queryPos = filePath.find('?');
    std::string pathWithoutQuery = (queryPos != std::string::npos) ? filePath.substr(0, queryPos) : filePath;
    // Check if the path ends with a slash (and is not just a single slash)
    if (pathWithoutQuery.length() > 1 && pathWithoutQuery[pathWithoutQuery.length() - 1] == '/') {
        // Return the path without the trailing slash
        return pathWithoutQuery.substr(0, pathWithoutQuery.length() - 1);
    }

    size_t pos = pathWithoutQuery.find_last_of("/\\");
    if (pos != std::string::npos) {
        // Check for dot after the last separator
        size_t dotPos = pathWithoutQuery.find_last_of('.');
        if (dotPos != std::string::npos && dotPos > pos) {
            // Remove filename and extension
            return pathWithoutQuery.substr(0, pos);
        } else {
            // No file extension, return the original path
            return pathWithoutQuery;
        }
    }
    // No directory separator found, treat as file without directory
    return pathWithoutQuery;
}

void RequestHandler::handleDeleteRequest(const Server& server) {
  Route route;
  std::string filePath = extractDirectoryPath(parser.getUri());
  try {
    route = server.getRoute(filePath);
  } catch (const std::out_of_range& e) {
    sendErrorResponse(404);
    Logger::log(ERROR, "404 - No route found for URI: " + parser.getUri());
    return;
  }

  filePath = getFilePathFromUri(route, parser.getUri());
  if (!route.getDeleteMethod()) {
    sendErrorResponse(405);
    Logger::log(ERROR, "405 - Method not allowed for URI: " + parser.getUri());
    return;
  }

  if (!ParsingUtils::doesPathExist(filePath)) {
    Logger::log(ERROR, "404 - File not found: " + filePath);
    sendErrorResponse(404);
    return;
  }

  std::string dir = extractDirectoryPath(filePath);

  //check if write and execute permissions are set in the directory in order to delete file.
  if (!ParsingUtils::hasWriteAndExecutePermissions(dir)) {
    Logger::log(ERROR, "403 - Insufficient permissions to delete file: " + dir);
    sendErrorResponse(403);
    return;
  }

  if (unlink(filePath.c_str()) != 0) {
    Logger::log(ERROR, "500 - Error deleting file: " + filePath);
    sendErrorResponse(500);
    return;
  }

  sendSuccessResponse("200 OK", "text/html", "File deleted successfully");
  return;
}

void RequestHandler::handleRequest(const Server& server) {
  Route route;
  if (parser.getMethod() == "GET") {
    handleGetRequest(server);
  }
  else if (parser.getMethod() == "POST") {
    handlePostRequest(server);
  }
  else if (parser.getMethod() == "DELETE")
    handleDeleteRequest(server);
}

std::string RequestHandler::getFilename(const MultipartFormDataParser& parser) {
  std::string filename = parser.getFileField("filename");
  if (filename.empty()) {
    filename = extractFilename(this->parser);
    if (filename.empty()) {
      filename = "untitled";
    }
  }
  return filename;
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
    write(client_fd, response.c_str(), response.size());

    std::cout << "Sent response with status code: " << statusCode << std::endl;
}

RequestHandler::RequestHandler() {}

void RequestHandler::closeConnection(void) {
  if (client_fd >= 0) {
    SystemUtils::closeUtil(client_fd);
    client_fd = -1; // Invalidate the file descriptor
  }
    delete this;
}

// bool RequestHandler::getShouldBeDeleted() const {
//   return shouldBeDeleted;
// }
