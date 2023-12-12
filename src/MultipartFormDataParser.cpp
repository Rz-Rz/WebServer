#include "MultipartFormDataParser.hpp"
#include <sstream>
#include <iostream>



void MultipartFormDataParser::parse() {
  std::vector<std::string> parts = splitBodyByBoundary();
  std::cout << "Number of parts: " << parts.size() << std::endl;
  for (size_t i = 0; i < parts.size(); ++i) {
    parsePart(parts[i]);
  }
}

void MultipartFormDataParser::parseDisposition(const std::string& disposition, std::string& name, std::string& filename) {
    std::istringstream stream(disposition);
    std::string segment;

    while (std::getline(stream, segment, ';')) {
        std::string::size_type pos;

        // Trim leading spaces
        pos = segment.find_first_not_of(" ");
        if (pos != std::string::npos) {
            segment = segment.substr(pos);
        }

        // Extract name
        pos = segment.find("name=\"");
        if (pos != std::string::npos) {
            name = segment.substr(pos + 6); // 6 is the length of 'name="'
            name = name.substr(0, name.find("\""));
        }

        // Extract filename
        pos = segment.find("filename=\"");
        if (pos != std::string::npos) {
            filename = segment.substr(pos + 10); // 10 is the length of 'filename="'
            filename = filename.substr(0, filename.find("\""));
        }
    }
}

std::vector<std::string> MultipartFormDataParser::splitBodyByBoundary() {
    std::vector<std::string> parts;
    std::string fullBoundary = boundary; // Using the received boundary
    std::string boundaryTerminator = fullBoundary + "--"; // Boundary terminator
    size_t minimumValidPartSize = fullBoundary.length() + 2; // 2 is the length of CRLF after the boundary
    size_t pos = 0;

    while ((pos = body.find(fullBoundary, pos)) != std::string::npos) {
        size_t start = pos + fullBoundary.length();
        if (body[start] == '\r' && body[start + 1] == '\n') {
            start += 2; // Skip CRLF after the boundary
        }

        pos = body.find(fullBoundary, start);
        if (pos == std::string::npos) {
            pos = body.find(boundaryTerminator, start); // Check for boundary terminator
        }

        std::string token = body.substr(start, pos - start);
        trim(token); // Remove leading and trailing whitespace/newlines

        // Skip parts that are too small to be valid (e.g., residual "--")
        if (!token.empty() && token.size() > minimumValidPartSize) { 
            parts.push_back(token);
        }
    }
    return parts;
}

void MultipartFormDataParser::parsePart(const std::string& part) {
  std::cout << "Parsing part, size: " << part.size() << std::endl;
  std::cout << "Part content: " << part << std::endl; // Be cautious with large parts
    // First, split the part into headers and content.
    std::string::size_type pos = part.find("\r\n\r\n");
    if (pos == std::string::npos) {
        // Handle error: part does not contain header-content separator
        throw MultipartFormDataParserException("Part does not contain header-content separator");
        return;
    }

    std::string headersPart = part.substr(0, pos);
    std::string contentPart = part.substr(pos + 4); // 4 is the length of "\r\n\r\n"

    // Parse headers
    std::map<std::string, std::string> headers = parseHeaders(headersPart);

    // Check for Content-Disposition header to determine if it's a file or a form field
    std::map<std::string, std::string>::iterator it = headers.find("Content-Disposition");
    if (it != headers.end()) {
        // Example Content-Disposition: form-data; name="fieldName"; filename="filename.jpg"
        std::string disposition = it->second;
        std::cout << "Content-Disposition: " << disposition << std::endl;

        // Parse the disposition to extract name and filename
        std::string name, filename;
        // You need to implement parseDisposition that parses the disposition string
        // and extracts name and filename
        parseDisposition(disposition, name, filename);
        std::cout << "name: " << name << std::endl;
        std::cout << "filename: " << filename << std::endl;

        if (!filename.empty()) {
            // It's a file field
            fileFields[name] = contentPart;
        } else {
            // It's a regular form field
            formFields[name] = contentPart;
        }
    } else {
        // Handle error: No Content-Disposition header found
        throw MultipartFormDataParserException("No Content-Disposition header found");
    }
}

std::map<std::string, std::string> MultipartFormDataParser::parseHeaders(const std::string& part) const {
  std::map<std::string, std::string> headers;
  std::istringstream stream(part);
  std::string line;

  while (std::getline(stream, line) && !line.empty()) {
    // Trim trailing carriage return if present
    if (*line.rbegin() == '\r') {
      line.erase(line.length() - 1);
    }

    std::string::size_type pos = line.find(":");
    if (pos != std::string::npos) {
      std::string headerName = line.substr(0, pos);
      std::string headerValue = line.substr(pos + 1);

      // Trim leading and trailing whitespaces from header name and value
      trim(headerName);
      trim(headerValue);

      headers[headerName] = headerValue;
    }
  }
  return headers;
}

void MultipartFormDataParser::trim(std::string& str) const {
    std::string::size_type pos = str.find_first_not_of(" \t");
    if (pos != std::string::npos) {
        str.erase(0, pos);
    }
    pos = str.find_last_not_of(" \t");
    if (pos != std::string::npos) {
        str.erase(pos + 1);
    }
}

std::string MultipartFormDataParser::extractContent(const std::string& part) const {
    std::string::size_type pos = part.find("\r\n\r\n");
    
    if (pos != std::string::npos) {
        // The content starts after "\r\n\r\n" which is 4 characters long
        return part.substr(pos + 4);
    } else {
        // If "\r\n\r\n" is not found, return an empty string or handle the error
        return "";
    }
}

std::map<std::string, std::string> MultipartFormDataParser::getFileFields() const {
    return fileFields;
}

std::map<std::string, std::string> MultipartFormDataParser::getFormFields() const {
    return formFields;
}

std::string MultipartFormDataParser::getFileField(const std::string& fieldName) const {
    std::map<std::string, std::string>::const_iterator it = fileFields.find(fieldName);
    if (it != fileFields.end()) {
        return it->second;
    } else {
        return "";
    }
}

std::string MultipartFormDataParser::getFormField(const std::string& fieldName) const {
    std::map<std::string, std::string>::const_iterator it = formFields.find(fieldName);
    if (it != formFields.end()) {
        return it->second;
    } else {
        return "";
    }
}

MultipartFormDataParser::MultipartFormDataParserException::MultipartFormDataParserException(const std::string& msg)
    : message(msg) {}

MultipartFormDataParser::MultipartFormDataParserException::~MultipartFormDataParserException() throw() {}

const char* MultipartFormDataParser::MultipartFormDataParserException::what() const throw() {
    return message.c_str();
}
