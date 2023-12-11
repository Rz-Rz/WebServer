#ifndef HTTPREQUESTPARSER_HPP
#define HTTPREQUESTPARSER_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "ParsingUtils.hpp"

class HTTPRequestParser {
private:
    std::string requestData;
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
    bool requestLineParsed;
    bool headersParsed;

    bool parseRequestLine(const std::string& requestLine);
    bool parseHeaders(void);
    void extractBody(void);

public:
    HTTPRequestParser() : requestLineParsed(false), headersParsed(false) {}

    void appendData(const std::string& data);

    std::string getMethod() const;
    std::string getUri() const;
    std::string getHttpVersion() const;
    std::string getHeader(const std::string& headerName) const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getBody() const;

    bool isRequestLineParsed() const;
    bool areHeadersParsed() const;

    void printHeaders() const;
};

#endif
