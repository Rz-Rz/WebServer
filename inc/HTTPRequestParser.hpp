#ifndef HTTPREQUESTPARSER_HPP
#define HTTPREQUESTPARSER_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "ParsingUtils.hpp"

class HTTPRequestParser {
private:
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
    size_t contentLength;
    bool isContentLengthParsed;
    bool requestLineParsed;
    bool headersParsed;
    bool isPostRequest;

    bool parseRequestLine(const std::string& requestLine);
    bool parseHeaders(void);
    void extractBody(void);
    void parseContentLength();

public:
    HTTPRequestParser();

    void appendData(const std::string& data);
    std::string requestData;

    std::string getMethod() const;
    std::string getUri() const;
    std::string getHttpVersion() const;
    std::string getHeader(const std::string& headerName) const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getBody() const;
    std::string getBoundary() const;

    bool isCompleteRequest() const;
    bool isRequestLineParsed() const;
    bool areHeadersParsed() const;

    void printHeaders() const;

    class InvalidHTTPVersionException : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "Invalid HTTP version";
        }
    };
    class InvalidMethodException : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "Invalid method";
        }
    };
};

#endif
