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
#include "ServerManager.hpp"
#include "Reactor.hpp"
#include <algorithm>

RequestHandler::RequestHandler(int fd, Reactor *reactor) : client_fd(fd), reactor(reactor) {}

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
          sendErrorResponse(505, NULL);
          closeConnection();
          break;
        }
        catch (const HTTPRequestParser::InvalidMethodException& e) {
          Logger::log(ERROR, "Error Parsing HTTP Request: " + std::string(e.what()));
          sendErrorResponse(405, NULL);
          closeConnection();
          break;
        }
        reactor->updateLastActivity(client_fd);
        // Check if the entire request has been received
        if (parser.isCompleteRequest()) {
          // std::cout << "PARSED DATA" << std::endl << parser.requestData << std::endl << "END PARSED DATA" << std::endl;
          Logger::log(INFO, "Received complete request");
          Server* server = findServerForHost(parser.getHeader("Host"), ServerManager::getInstance().getServersMap());
          if (server == NULL)
          {
            Logger::log(ERROR, "No matching server found for request:" + parser.getUri());
            sendErrorResponse(400, NULL);
            closeConnection();
          }
          else {
            RequestHandler::handleRequest(server);
            if (shouldCloseConnection()) {
              Logger::log(INFO, "Should close connection");
              closeConnection();
            }
          }
          break;
        }
      }
      else if (bytes_read == 0) {
        // Client disconnected
        Logger::log(INFO, "Client disconnected");
        closeConnection();
        break;
      }
      else {
        Logger::log(ERROR, "Error reading from client: ");
        closeConnection();
        // We cannot check the error code because of the project rules (wtf?)
        // Logger::log(ERROR, "Error reading from client: " + std::string(strerror(errno)));
        break;
      }
    }
  }
}

Server* RequestHandler::findServerForHost(const std::string& host, const std::map<std::string, Server*>* serversMap) {
    // Extract hostname or IP and port from the host header
    std::string parsedHost;
    int parsedPort = -1; // Default to an invalid port
    size_t colonPos = host.find(':');
    if (colonPos != std::string::npos) {
        parsedHost = host.substr(0, colonPos);
        std::istringstream iss(host.substr(colonPos + 1));
        iss >> parsedPort;
    } else {
        parsedHost = host;
    }

    // Check if the host header is an IP address
    bool isIpAddress = ParsingUtils::isValidIPv4(parsedHost);

    if (!serversMap) {
        Logger::log(ERROR, "Servers map is NULL");
        return NULL;
    }

    for (std::map<std::string, Server*>::const_iterator it = serversMap->begin(); it != serversMap->end(); ++it) {
        Server* server = it->second;
        if (server != NULL) {
            const std::string& serverHost = server->getHost();
            const std::vector<int>& serverPorts = server->getPorts();

            // Check for IP address match or empty host
            if (isIpAddress && (serverHost.empty() || parsedHost == serverHost)) {
                // Check if parsedPort is among the server's ports
                if (std::find(serverPorts.begin(), serverPorts.end(), parsedPort) != serverPorts.end()) {
                    std::ostringstream oss;
                    oss << parsedPort;
                    Logger::log(INFO, "Found matching server for IP address: " + parsedHost + " on port " + oss.str() + " with server :" + server->getServerName());
                    return server;
                }
            }
            // Check for hostname match
            else if (!isIpAddress && parsedHost == server->getServerName()) {
                Logger::log(INFO, "Found matching server for host name: " + parsedHost + " with server :" + server->getServerName());
                return server;
            }
        }
    }
    Logger::log(ERROR, "No matching server found for host: " + parsedHost);
    return NULL;
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
  if (route.getHasCGI())
  {
    filePath = route.getCGIPath();
    return filePath;
  }
  // Append the URI to the root directory path, omitting the leading slash in the URI
  if (route.getHasDefaultFile()) {
    std::string file = route.getDefaultFile();
    if (file[0] == '/')
      filePath += file.substr(1);
    else
      filePath += file;
  }
  else if (!route.getHasRootDirectoryPath())
  {
    filePath += uri.substr(1);
  }
  else
  {
    filePath += uri;
  }
  return filePath;
}


void RequestHandler::sendErrorResponse(int errorCode, const Server* server) {
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

void RequestHandler::handleDirectoryRequest(const Route& route, const Server* server)
{
      std::string directoryPath = getFilePathFromUri(route, parser.getUri());
      if (!ParsingUtils::doesPathExist(directoryPath)) {
        Logger::log(ERROR, "Directory does not exist: " + directoryPath);
        sendErrorResponse(403, server);
        return;
      }
      if (!ParsingUtils::hasReadPermissions(directoryPath)) {
        Logger::log(ERROR, "Directory is not readable: " + directoryPath);
        sendErrorResponse(403, server);
        return;
      }
      // Directory exists and is readable
      Logger::log(INFO, "Directory listing on GET request: " + directoryPath);
      std::vector<std::string> contents;
      try {
        contents = ParsingUtils::getDirectoryContents(directoryPath);
      } catch (const std::exception& e) {
        Logger::log(ERROR, "500 - Error reading directory contents: " + std::string(e.what()));
        sendErrorResponse(500, server);
        return;
      }
      std::string directoryListingPage = generateDirectoryListingPage(contents, parser.getUri());
      sendSuccessResponse("200 OK", "text/html", directoryListingPage);
      return;
}

std::string RequestHandler::getMimeType(const std::string& filePath) {
    Logger::log(DEBUG, "getMimeType called with filePath: " + filePath);

    // Local map for MIME types
    std::map<std::string, std::string> localMimeTypes;
    localMimeTypes["html"] = "text/html";
    localMimeTypes["css"] = "text/css";
    localMimeTypes["jpg"] = "image/jpeg";
    localMimeTypes["jpeg"] = "image/jpeg";
    // Add more MIME types as needed

    size_t dotPos = filePath.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string extension = filePath.substr(dotPos + 1);
        Logger::log(DEBUG, "Extracted file extension: " + extension);

        std::ostringstream oss;
        oss << localMimeTypes.size();
        Logger::log(DEBUG, "Local MimeTypes map size: " + oss.str());

        std::map<std::string, std::string>::const_iterator it = localMimeTypes.find(extension);
        if (it != localMimeTypes.end()) {
            Logger::log(DEBUG, "MIME type found: " + it->second);
            return it->second;
        }
    }

    Logger::log(WARNING, "Returning default MIME type for extension: " + filePath.substr(dotPos + 1));
    return "application/octet-stream";
}

void RequestHandler::handleFileRequest(const Route& route, const Server* server) {
  std::string filePath = getFilePathFromUri(route, parser.getUri());
  std::cout << "File path after getFilePathFromUri: " << filePath << std::endl;
  if (ParsingUtils::isDirectory(filePath))
  {
    sendErrorResponse(403, server);
    Logger::log(ERROR, "403 - Directory listing is not enabled: " + filePath);
    return;
  }
  if (ParsingUtils::doesPathExistAndReadable(filePath)) {
    std::string fileContent = ParsingUtils::readFile(filePath);
    sendSuccessResponse("200 OK", getMimeType(filePath), fileContent);
    Logger::log(INFO, "File request on GET request: " + filePath); 
    return;
  } else {
    sendErrorResponse(404, server);
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

void RequestHandler::handleGetRequest(const Server* server) {
  std::string originalPath = removeQueryString(parser.getUri()); // Get the original URI
  std::cout << "originalPath after removeQueryString: " << originalPath << std::endl;
  Route route;
  try {
    // First try to find the route directly with the original URI
    route = server->getRoute(originalPath);
  } catch (const std::out_of_range&) {
    // If not found, try extracting the directory path and then finding the route
    std::string directoryPath = extractDirectoryPath(originalPath);
    std::cout << "directoryPath after extractDirectoryPath: " << directoryPath << std::endl;
    try {
      route = server->getRoute(directoryPath);
    } catch (const std::out_of_range& e) {
      // No route found for either the original URI or the directory path
      sendErrorResponse(404, server);
      Logger::log(ERROR, "404 - No route found for URI: " + originalPath);
      return;
    }
  }


  if (!route.getGetMethod()) {
    // Method not allowed for this route
    sendErrorResponse(405, server);
    Logger::log(ERROR, "405 - Method not allowed for URI: " + parser.getUri());
    return;
  }
  if (route.getRedirect()) {
    handleRedirect(route);
    return;
  }
  else if (route.getDirectoryListing() && !route.getHasDefaultFile()) {
    handleDirectoryRequest(route, server);
    return;
  }
  else if (route.getHasCGI()) {
    handleCGIRequest(route, server);
    return;
  }
  else {
    handleFileRequest(route, server);
    return;
  }
}

void RequestHandler::setCGIEnvironment(const std::string& queryString) {
  if (queryString.empty()) {
    return;
  }
  setenv("QUERY_STRING", queryString.c_str(), 1);
}

std::string RequestHandler::removeQueryString(const std::string& uri) {
  size_t pos = uri.find('?');
  if (pos != std::string::npos) {
    return uri.substr(0, pos);
  }
  return uri;
}

void RequestHandler::handleCGIRequest(const Route& route, const Server* server) {
  std::string queryString = extractQueryString(parser.getUri());
  std::string filePath = getFilePathFromUri(route, parser.getUri());
  std::cout << "File path after getFilePathFromUri: " << filePath << std::endl;
  filePath = removeQueryString(filePath);
  std::cout << "File path after removeQueryString: " << filePath << std::endl;

  if (!ParsingUtils::doesPathExist(filePath)) {
    Logger::log(ERROR, "404 - File not found: " + filePath);
    sendErrorResponse(404, server);
    return;
  }

  if (!ParsingUtils::hasExecutePermissions(filePath)) {
    Logger::log(ERROR, "403 - File is not executable: " + filePath);
    sendErrorResponse(403, server);
    return;
  }
  std::cout << "queryString : " << queryString << std::endl;
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

bool RequestHandler::isPayloadTooLarge(const Server* server, const Route& route) {
    std::string contentLengthHeader = parser.getHeader("Content-Length");
    long long contentLength = 0;
    if (!contentLengthHeader.empty()) {
        contentLength = std::atoll(contentLengthHeader.c_str());
        if (route.getHasMaxBodySize())
        {
          if (contentLength > route.getMaxBodySize()) {
            sendErrorResponse(413, server);
            Logger::log(ERROR, "413 - Payload is too large: " + contentLengthHeader + " Maximum allowed: " + ParsingUtils::toString(route.getMaxBodySize()) + " bytes");
            return true; // Payload is too large
          }
        }
        if (contentLength > server->getMaxClientBodySize()) {
            sendErrorResponse(413, server);
            Logger::log(ERROR, "413 - Payload is too large: " + contentLengthHeader + " Maximum allowed: " + ParsingUtils::toString(server->getMaxClientBodySize()) + " bytes");
            return true; // Payload is too large
        }
    }
    return false; // Payload is within the acceptable size
}

std::string RequestHandler::extractFilename(const HTTPRequestParser& parser) {
    std::string uri = parser.getUri();

    // Check if the URI ends with a slash - indicating a directory
    if (!uri.empty() && uri.length() - 1 == '/') {
        return "";
    }

    // Find the last slash position
    size_t lastSlashPos = uri.find_last_of('/');
    // Find the last dot position
    size_t lastDotPos = uri.find_last_of('.');

    // Check if a dot exists after the last slash - indicating a file
    if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastDotPos > lastSlashPos) {
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

void RequestHandler::handleFileUpload(const Route& route, const Server* server) {
  std::string filePath = getUploadDirectoryFromUri(route, parser.getUri());
  bool multipartError = false;

  std::string body = parser.getBody();
  std::string boundary = parser.getBoundary();

  if (boundary.empty()) {
    Logger::log(ERROR, "400 - No boundary found in multipart form data");
    sendErrorResponse(400, server); // Bad Request
    return;
  }

  MultipartFormDataParser multipartParser(body, boundary);
  try {
    multipartParser.parse();
  } catch (const MultipartFormDataParser::MultipartFormDataParserException& e) {
    Logger::log(ERROR, std::string("Error parsing multipart form data: ") + e.what());
    multipartError = true;
  }

  // Extracting the filename and file content
  if (!multipartError)
  {
    const std::map<std::string, std::string>& fileFields = multipartParser.getFileFields();
    if (fileFields.empty()) {
      Logger::log(ERROR, "No files found in the request");
    }
  }

  if (!ParsingUtils::doesPathExist(filePath)) {
    Logger::log(ERROR, "Directory does not exist: " + filePath);
    sendErrorResponse(403, server);
    return;
  }

  if (!ParsingUtils::hasWritePermissions(filePath)) {
    Logger::log(ERROR, "Directory is not writable: " + filePath);
    sendErrorResponse(403, server);
    return;
  }

  filePath += getFilename(multipartParser);
  // Directory exists and is writable
  Logger::log(INFO, "File upload on POST request: " + filePath);
  std::string fileContent = parser.getBody();
  std::ofstream fileStream(filePath.c_str(), std::ios::out | std::ios::binary);
  if (!fileStream) {
    Logger::log(ERROR, "500 - Error opening file for writing: " + filePath);
    sendErrorResponse(500, server);
    return;
  }
  fileStream << fileContent;
  fileStream.close();
  sendSuccessResponse("200 OK", "text/html", "File uploaded successfully");
  return;
}

void RequestHandler::handlePostRequest(const Server* server) {
  Route route;
  
  try {
    route = server->getRoute(parser.getUri());
  } catch (const std::out_of_range& e) {
    sendErrorResponse(404, server);
    Logger::log(ERROR, "404 - No route found for URI: " + parser.getUri());
    return;
  }

  if (!route.getPostMethod()) {
    sendErrorResponse(405, server);
    Logger::log(ERROR, "405 - Method not allowed for URI: " + parser.getUri());
    return;
  }

  if (isPayloadTooLarge(server, route)) {
    return;
  }

  if (route.getAllowFileUpload()) {
    handleFileUpload(route, server);
  }
  else {
    Logger::log(INFO, "POST request on URI: " + parser.getUri());
    sendSuccessResponse("200 OK", "text/html", "POST request received with body: " + parser.getBody());
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

void RequestHandler::handleDeleteRequest(const Server* server) {
  Route route;
  std::string filePath = extractDirectoryPath(parser.getUri());
  try {
    route = server->getRoute(filePath);
  } catch (const std::out_of_range& e) {
    sendErrorResponse(404, server);
    Logger::log(ERROR, "404 - No route found for URI: " + parser.getUri());
    return;
  }

  filePath = getFilePathFromUri(route, parser.getUri());
  if (!route.getDeleteMethod()) {
    sendErrorResponse(405, server);
    Logger::log(ERROR, "405 - Method not allowed for URI: " + parser.getUri());
    return;
  }

  if (!ParsingUtils::doesPathExist(filePath)) {
    Logger::log(ERROR, "404 - File not found: " + filePath);
    sendErrorResponse(404, server);
    return;
  }

  std::string dir = extractDirectoryPath(filePath);

  //check if write and execute permissions are set in the directory in order to delete file.
  if (!ParsingUtils::hasWriteAndExecutePermissions(dir)) {
    Logger::log(ERROR, "403 - Insufficient permissions to delete file: " + dir);
    sendErrorResponse(403, server);
    return;
  }

  if (unlink(filePath.c_str()) != 0) {
    Logger::log(ERROR, "500 - Error deleting file: " + filePath);
    sendErrorResponse(500, server);
    return;
  }

  sendSuccessResponse("200 OK", "text/html", "File deleted successfully");
  return;
}

void RequestHandler::handleRequest(const Server* server) {
  if (server == NULL) {
    Logger::log(ERROR, "Server is NULL");
    return;
  }
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
      filename = "untitled.txt";
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
    if (write(client_fd, response.c_str(), response.size()) <= 0)
      Logger::log(ERROR, "Error sending redirect response: " + std::string(strerror(errno)));
    else
      Logger::log(INFO, "Sent redirect response to: " + redirectLocation);
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
    if (write(client_fd, response.c_str(), response.size()) <= 0)
      Logger::log(ERROR, "Error sending response: " + std::string(strerror(errno)));
    else
      Logger::log(INFO, "Sent response with status code: " + statusCode);
}

RequestHandler::RequestHandler() {}

bool RequestHandler::shouldCloseConnection() {
    return reactor->isClientInactive(client_fd);
}

void RequestHandler::closeConnection(void) {
  reactor->deregisterHandler(client_fd);
  if (client_fd >= 0) {
    SystemUtils::closeUtil(client_fd);
    client_fd = -1;
  }
  delete this;
}
