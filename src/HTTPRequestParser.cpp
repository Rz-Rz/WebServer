#include "HTTPRequestParser.hpp"
#include "Logger.hpp"

bool HTTPRequestParser::parseRequestLine(const std::string& requestLine) {
  std::istringstream iss(requestLine);
  if (!(iss >> method >> uri >> httpVersion)) {
    return false;
  }
  ParsingUtils::trim(method);
  ParsingUtils::trim(uri);
  ParsingUtils::trim(httpVersion);
  return true;
}

void HTTPRequestParser::appendData(const std::string& data) {
  requestData.append(data);
  if (!requestLineParsed) {
    size_t lineEnd = requestData.find("\r\n");
    if (lineEnd != std::string::npos) {
      if (parseRequestLine(requestData.substr(0, lineEnd))) {
        requestLineParsed = true;
        requestData.erase(0, lineEnd + 2);
      }
    }
  }
  if (requestLineParsed && !headersParsed) {
    if (requestData.find("\r\n\r\n") != std::string::npos) {
      parseHeaders();
      extractBody();
    }
  }
}

bool HTTPRequestParser::parseHeaders() {
  std::istringstream iss(requestData);
  std::string line;
  while (std::getline(iss, line) && line != "\r") {
    size_t delimiterPos = line.find(":");
    if (delimiterPos != std::string::npos) {
      std::string key = line.substr(0, delimiterPos);
      std::string value = line.substr(delimiterPos + 1);
      ParsingUtils::trimAndLower(key);
      ParsingUtils::trim(value);
      headers[key] = value;
      std::cout << "key: " << key << " value: " << value << std::endl;
    } else {
      Logger::log(ERROR, "Invalid header line: " + line);
      return false;
    }
  }
  headersParsed = true;
  return true;
}

void HTTPRequestParser::extractBody() {
  // The body starts after the headers (which ends with a blank line)
  size_t headersEnd = requestData.find("\r\n\r\n");
  if (headersEnd != std::string::npos) {
    body = requestData.substr(headersEnd + 4);
  }
}

std::string HTTPRequestParser::getMethod() const {
  return method;
}

std::string HTTPRequestParser::getUri() const {
  return uri;
}

std::string HTTPRequestParser::getHttpVersion() const {
  return httpVersion;
}

std::string HTTPRequestParser::getHeader(const std::string& headerName) const {
  std::map<std::string, std::string>::const_iterator it = headers.find(headerName);
  if (it != headers.end()) {
    return it->second;
  }
  else {
    return "";
  }
}

std::map<std::string, std::string> HTTPRequestParser::getHeaders() const {
  return headers;
}

std::string HTTPRequestParser::getBody() const {
  return body;
}

bool HTTPRequestParser::isRequestLineParsed() const {
  return requestLineParsed;
}

bool HTTPRequestParser::areHeadersParsed() const {
  return headersParsed;
}

void HTTPRequestParser::printHeaders() const {
  std::map<std::string, std::string> hdrs = getHeaders();
  for (std::map<std::string, std::string>::const_iterator it = hdrs.begin(); it != hdrs.end(); ++it) {
    std::cout << it->first << ": " << it->second << std::endl;
  }
}

std::string HTTPRequestParser::getBoundary() const {
    std::map<std::string, std::string>::const_iterator it = headers.find("content-type");
    if (it != headers.end()) {
        const std::string& contentType = it->second;
        std::cout << "Content-Type: " << contentType << std::endl;
        std::istringstream stream(contentType);
        std::string segment;

        while (std::getline(stream, segment, ';')) {
            std::size_t pos = segment.find("boundary=");
            if (pos != std::string::npos) {
                std::string boundary = segment.substr(pos + 9); // 9 is the length of "boundary="

                // Remove leading whitespace
                std::size_t start = boundary.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    boundary = boundary.substr(start);
                }

                // Remove trailing whitespace
                std::size_t end = boundary.find_last_not_of(" \t");
                if (end != std::string::npos) {
                    boundary = boundary.substr(0, end + 1);
                }

                // Remove quotes if any
                if (boundary.size() >= 2 && boundary[0] == '"' && boundary[boundary.size() - 1] == '"') {
                    boundary = boundary.substr(1, boundary.size() - 2);
                }

                return boundary;
            }
        }
    }
    return ""; // Return empty string if boundary is not found
}
