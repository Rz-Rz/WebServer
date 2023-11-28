#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <set>
#include <string>
#include <vector>


class Route {
public:
    Route();
    void setRoutePath(const std::string& path);
    void setGetMethod(const bool value);
    void setPostMethod(const bool value);
    void setRedirectLocation(const std::string& redirect);
    void setHasRedirect(const bool value);
    void setRootDirectoryPath(const std::string& path);
    void setDirectoryListing(bool listing);
    void setDefaultFile(const std::string& file);
    void setHasCGI(bool value);
    void setCgiExtensions(const std::vector<std::string>& extensions);
    void setAllowFileUpload(bool allow);
    void setUploadLocation(const std::string& location);

    std::string getRoutePath() const;
    bool getGetMethod() const;
    bool getPostMethod() const;
    std::string getRedirectLocation() const;
    bool getHasRedirect() const;
    std::string getRootDirectoryPath() const;
    bool getDirectoryListing() const;
    std::string getDefaultFile() const;
    bool getHasCGI() const;
    std::vector<std::string> getCgiExtensions() const;
    bool getAllowFileUpload() const;
    std::string getUploadLocation() const;

private:
    std::string routePath;
    bool getMethod;
    bool postMethod;
    bool has_redirect;
    std::string redirect;
    std::string rootDirectoryPath;
    bool directoryListing;
    std::string defaultFile;
    bool has_CGI;
    std::vector<std::string> cgiExtensions;
    bool allowFileUpload;
    std::string uploadLocation;
};

#endif
