#ifndef ERRORPAGEMANAGER_HPP
#define ERRORPAGEMANAGER_HPP

#include <string>
#include <map>

class ErrorPageManager {
public:
    ErrorPageManager(const std::string& defaultPagePath);
    void setErrorPage(int errorCode, const std::string& pagePath);
    std::string getErrorPage(int errorCode) const;
    std::string getDefaultErrorPage() const;
    std::string getFormattedErrorPage(int errorCode) const;

private:
    std::map<int, std::string> customErrorPages;
    std::string defaultErrorPagePath;
};

#endif
