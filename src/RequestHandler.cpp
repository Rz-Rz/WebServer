#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sys/epoll.h>
#include <errno.h>
#include <cstdlib>
#include <algorithm>
#include "RequestHandler.hpp"
#include "ServerManager.hpp"
#include "Reactor.hpp"
#include "HTTPResponse.hpp"
#include "MultipartFormDataParser.hpp"
#include "SystemUtils.hpp"
#include "HTTPRequestParser.hpp"
#include "Logger.hpp"
#include "ErrorPageManager.hpp"
#include "ParsingUtils.hpp"
#include "CgiHandler.hpp"

RequestHandler::RequestHandler(int fd, Reactor *reactor) : reactor(reactor), closeConnectionFlag (true) {
  EventHandler::setHandle(fd);
}

RequestHandler::~RequestHandler() {}

void RequestHandler::handleSession() {
  SessionManager& sessionManager = ServerManager::getInstance().getSessionManager();
  std::string cookieHeader = parser.getHeader("Cookie");
  Logger::log(INFO, "Cookie header: " + cookieHeader);
  if (!cookieHeader.empty()) {
    std::string sessionId = extractSessionIdFromCookie(cookieHeader);
    Logger::log(INFO, "Session ID: " + sessionId);
    SessionData* sessionData = sessionManager.getSessionData(sessionId);
    if (sessionData != NULL) {
      sessionData->incrementRequestCount();
    }
    else {
      // A cookie already exist so let's register it and use it
      sessionId = sessionManager.createSession(sessionId);
      cookie = Cookie("session_id", sessionId);
    }
  } else {
    std::string sessionId = sessionManager.createSession();
    // Initialize cookie with the new session ID
    cookie = Cookie("session_id", sessionId);
  }
}

void RequestHandler::handleEvent(uint32_t events) {
  if (events & EPOLLIN) {
    char buffer[1024];

    while (true) {
      ssize_t bytes_read = read(EventHandler::getHandle(), buffer, sizeof(buffer));
      if (bytes_read > 0) {
        try {
          parser.appendData(std::string(buffer, bytes_read));
          // std::cout << "PACKET RECV ----" << std::endl << std::string(buffer, bytes_read) << std::cout << "PACKET END ----" << std::endl;
        } catch (const HTTPRequestParser::InvalidHTTPVersionException& e) {
          Logger::log(ERROR, std::string("Error Parsing HTTP Request: ") + e.what());
          HTTPResponse::sendErrorResponse(505, NULL, EventHandler::getHandle());
          closeConnection();
          break;
        }
        catch (const HTTPRequestParser::InvalidMethodException& e) {
          Logger::log(ERROR, "Error Parsing HTTP Request: " + std::string(e.what()));
          HTTPResponse::sendErrorResponse(405, NULL, EventHandler::getHandle());
          closeConnection();
          break;
        }
        reactor->updateLastActivity(EventHandler::getHandle());
        // Check if the entire request has been received
        if (parser.isCompleteRequest()) {
          // std::cout << "PARSED DATA" << std::endl << parser.requestData << std::endl << "END PARSED DATA" << std::endl;
          Logger::log(INFO, "Received complete request");
          Server* server = findServerForHost(parser.getHeader("Host"), ServerManager::getInstance().getServersMap());
          if (server == NULL)
          {
            Logger::log(ERROR, "No matching server found for request:" + parser.getUri());
            HTTPResponse::sendErrorResponse(400, NULL, EventHandler::getHandle());
            closeConnection();
          }
          else {
            handleSession();
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


std::string RequestHandler::getFilePathFromUri(const Route& route, const std::string& uri) {
	std::string rootPath = route.getRootDirectoryPath();
	std::string filePath = ParsingUtils::removeFinalSlash(rootPath);
	filePath = filePath + uri;
	if (ParsingUtils::isDirectory(filePath))
	{
		if (route.getHasDefaultFile()) {
			std::string file = route.getDefaultFile();
      Logger::log(INFO, "Serving default file: " + file + " for URI: " + uri);
			if (file[0] == '/')
				filePath += file.substr(1);
			else
				filePath += file;
		}
		if (!rootPath.empty() && rootPath[rootPath.length() - 1] != '/') {
			filePath += "/";
		}
	}
	if (route.getHasCGI())
	{
		filePath = route.getCGIPath();
    Logger::log(INFO, "Serving CGI file: " + filePath + " for URI: " + uri);
		return filePath;
	}
	return filePath;
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
  Logger::log(INFO, "Redirecting to: " + route.getRedirectLocation());
  HTTPResponse::sendRedirectResponse(route.getRedirectLocation(), EventHandler::getHandle());
}

void RequestHandler::handleDirectoryRequest(const Route& route, const Server* server)
{
      std::string directoryPath = getFilePathFromUri(route, parser.getUri());
      if (!ParsingUtils::doesPathExist(directoryPath)) {
        Logger::log(ERROR, "Directory does not exist: " + directoryPath);
        HTTPResponse::sendErrorResponse(403, server, EventHandler::getHandle());
        return;
      }
      if (!ParsingUtils::hasReadPermissions(directoryPath)) {
        Logger::log(ERROR, "Directory is not readable: " + directoryPath);
        HTTPResponse::sendErrorResponse(403, server, EventHandler::getHandle());
        return;
      }
      // Directory exists and is readable
      Logger::log(INFO, "Directory listing on GET request: " + directoryPath);
      std::vector<std::string> contents;
      try {
        contents = ParsingUtils::getDirectoryContents(directoryPath);
      } catch (const std::exception& e) {
        Logger::log(ERROR, "500 - Error reading directory contents: " + std::string(e.what()));
        HTTPResponse::sendErrorResponse(500, server, EventHandler::getHandle());
        return;
      }
      std::string directoryListingPage = generateDirectoryListingPage(contents, parser.getUri());
      HTTPResponse::sendSuccessResponse("200 OK", "text/html", directoryListingPage, cookie, EventHandler::getHandle());
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

        std::ostringstream oss;
        oss << localMimeTypes.size();

        std::map<std::string, std::string>::const_iterator it = localMimeTypes.find(extension);
        if (it != localMimeTypes.end()) {
            Logger::log(INFO, "MIME type found: " + it->second);
            return it->second;
        }
    }

    Logger::log(INFO, "Returning default MIME type for extension: " + filePath.substr(dotPos + 1));
    return "application/octet-stream";
}

void RequestHandler::handleFileRequest(const Route& route, const Server* server) {
  std::string filePath = getFilePathFromUri(route, parser.getUri());
  Logger::log(INFO, "Looking to GET: " + filePath);
  if (ParsingUtils::isDirectory(filePath))
  {
    HTTPResponse::sendErrorResponse(403, server, EventHandler::getHandle());
    Logger::log(ERROR, "403 - Directory listing is not enabled: " + filePath);
    return;
  }
  if (ParsingUtils::doesPathExistAndReadable(filePath)) {
    std::string fileContent = ParsingUtils::readFile(filePath);
    // handle file with session
    SessionManager& sessionManager = ServerManager::getInstance().getSessionManager();
    std::string cookieHeader = parser.getHeader("Cookie");
    if (!cookieHeader.empty()) {
      std::string sessionId = extractSessionIdFromCookie(cookieHeader);
      SessionData* sessionData = sessionManager.getSessionData(sessionId);
      if (sessionData != NULL)
        fileContent = HTTPResponse::modifyHtmlContentForSession(fileContent, sessionData);
    }
    ServerManager::getInstance().getSessionManager().debugPrintSessions();
    HTTPResponse::sendSuccessResponse("200 OK", getMimeType(filePath), fileContent, cookie, EventHandler::getHandle());
    Logger::log(INFO, "File request on GET request: " + filePath); 
    return;
  } else {
    HTTPResponse::sendErrorResponse(404, server, EventHandler::getHandle());
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
  Route route;
  try {
    // First try to find the route directly with the original URI
    route = server->getRoute(originalPath);
  } catch (const std::out_of_range&) {
    // If not found, try extracting the directory path and then finding the route
    std::string directoryPath = extractDirectoryPath(originalPath);
    try {
      route = server->getRoute(directoryPath);
    } catch (const std::out_of_range& e) {
      // No route found for either the original URI or the directory path
      HTTPResponse::sendErrorResponse(404, server, EventHandler::getHandle());
      Logger::log(ERROR, "404 - No route found for URI: " + originalPath);
      return;
    }
  }


  if (!route.getGetMethod()) {
    // Method not allowed for this route
    HTTPResponse::sendErrorResponse(405, server, EventHandler::getHandle());
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
  filePath = removeQueryString(filePath);
  Logger::log(INFO, "Looking for: " + filePath);
  Logger::log(INFO, "Query string: " + queryString);

  if (!ParsingUtils::doesPathExist(filePath)) {
    Logger::log(ERROR, "404 - File not found: " + filePath);
    HTTPResponse::sendErrorResponse(404, server, EventHandler::getHandle());
    return;
  }

  if (!ParsingUtils::hasExecutePermissions(filePath)) {
    Logger::log(ERROR, "403 - File is not executable: " + filePath);
    HTTPResponse::sendErrorResponse(403, server, EventHandler::getHandle());
    return;
  }
  CgiHandler* cgiHandler = new CgiHandler(filePath, queryString, EventHandler::getHandle(), reactor);
  closeConnectionFlag = false;
  // File exists and is readable and executable
  Logger::log(INFO, "CGI request on GET request: " + filePath);
  reactor->registerHandler(cgiHandler);
  return;
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
            HTTPResponse::sendErrorResponse(413, server, EventHandler::getHandle());
            Logger::log(ERROR, "413 - Payload is too large: " + contentLengthHeader + " Maximum allowed: " + ParsingUtils::toString(route.getMaxBodySize()) + " bytes");
            return true; // Payload is too large
          }
        }
        if (contentLength > server->getMaxClientBodySize()) {
            HTTPResponse::sendErrorResponse(413, server, EventHandler::getHandle());
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
    HTTPResponse::sendErrorResponse(400, server, EventHandler::getHandle()); // Bad Request
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
    HTTPResponse::sendErrorResponse(403, server, EventHandler::getHandle());
    return;
  }

  if (!ParsingUtils::hasWritePermissions(filePath)) {
    Logger::log(ERROR, "Directory is not writable: " + filePath);
    HTTPResponse::sendErrorResponse(403, server, EventHandler::getHandle());
    return;
  }

  filePath += getFilename(multipartParser);
  // Directory exists and is writable
  Logger::log(INFO, "File upload on POST request: " + filePath);
  std::string fileContent = parser.getBody();
  std::ofstream fileStream(filePath.c_str(), std::ios::out | std::ios::binary);
  if (!fileStream) {
    Logger::log(ERROR, "500 - Error opening file for writing: " + filePath);
    HTTPResponse::sendErrorResponse(500, server, EventHandler::getHandle());
    return;
  }
  fileStream << fileContent;
  fileStream.close();
  std::string successPageHtml = 
	  "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>Upload Success</title></head><body>"
	  "<h1>Upload Successful</h1><p>200 OK - Your file has been uploaded successfully.</p>"
	  "</body></html>";
  HTTPResponse::sendSuccessResponse("200 OK", "text/html", successPageHtml, cookie, EventHandler::getHandle());
  return;
}

void RequestHandler::handlePostRequest(const Server* server) {
  Route route;
  
  try {
    route = server->getRoute(parser.getUri());
  } catch (const std::out_of_range& e) {
    HTTPResponse::sendErrorResponse(404, server, EventHandler::getHandle());
    Logger::log(ERROR, "404 - No route found for URI: " + parser.getUri());
    return;
  }

  if (!route.getPostMethod()) {
    HTTPResponse::sendErrorResponse(405, server, EventHandler::getHandle());
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
    HTTPResponse::sendSuccessResponse("200 OK", "text/html", " 200 OK - POST request received with body: " + parser.getBody(), cookie, EventHandler::getHandle());
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
	std::string originalPath = removeQueryString(parser.getUri()); // Get the original URI
	Route route;
	try {
		// First try to find the route directly with the original URI
		route = server->getRoute(originalPath);
	} catch (const std::out_of_range&) {
		// If not found, try extracting the directory path and then finding the route
		std::string directoryPath = extractDirectoryPath(originalPath);
		try {
			route = server->getRoute(directoryPath);
		} catch (const std::out_of_range& e) {
			// No route found for either the original URI or the directory path
			HTTPResponse::sendErrorResponse(404, server, EventHandler::getHandle());
			Logger::log(ERROR, "404 - No route found for URI: " + originalPath);
			return;
		}
	}

	std::string filePath = getFilePathFromUri(route, parser.getUri());
  Logger::log(INFO, "Looking to DELETE: " + filePath);
	if (!route.getDeleteMethod()) {
		HTTPResponse::sendErrorResponse(405, server, EventHandler::getHandle());
		Logger::log(ERROR, "405 - Method not allowed for URI: " + parser.getUri());
		return;
	}

	if (!ParsingUtils::doesPathExist(filePath)) {
		Logger::log(ERROR, "404 - File not found: " + filePath);
		HTTPResponse::sendErrorResponse(404, server, EventHandler::getHandle());
		return;
	}

	std::string dir = extractDirectoryPath(filePath);

	//check if write and execute permissions are set in the directory in order to delete file.
	if (!ParsingUtils::hasWriteAndExecutePermissions(dir)) {
		Logger::log(ERROR, "403 - Insufficient permissions to delete file: " + dir);
		HTTPResponse::sendErrorResponse(403, server, EventHandler::getHandle());
		return;
	}

	if (unlink(filePath.c_str()) != 0) {
		Logger::log(ERROR, "500 - Error deleting file: " + filePath);
		HTTPResponse::sendErrorResponse(500, server, EventHandler::getHandle());
		return;
	}

	HTTPResponse::sendSuccessResponse("200 OK", "text/html", "200 - OK File deleted successfully", cookie, EventHandler::getHandle());
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

RequestHandler::RequestHandler() {}

std::string RequestHandler::extractSessionIdFromCookie(const std::string& cookie) {
  size_t pos = cookie.find('=');
  if (pos != std::string::npos) {
    return cookie.substr(pos + 1);
  }
  return "";
}

bool RequestHandler::shouldCloseConnection() {
    return reactor->isClientInactive(EventHandler::getHandle());
}

void RequestHandler::closeConnection(void) {
  if (closeConnectionFlag)
  {
    reactor->removeFromInactivityList(EventHandler::getHandle());
    reactor->deregisterHandler(EventHandler::getHandle());
    if (EventHandler::getHandle() >= 0) {
      SystemUtils::closeUtil(EventHandler::getHandle());
    }
  }
  delete this;
}
