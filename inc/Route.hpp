#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <set>
#include <string>
#include <vector>


class Route {
public:
    Route();
    void setRoutePath(const std::string& path);
    void setMethods(const std::set<std::string>& methods);
    void setRedirect(const std::string& redirect);
    void setRootDirectoryPath(const std::string& path);
    void setDirectoryListing(bool listing);
    void setDefaultFile(const std::string& file);
    void setCgiExtensions(const std::vector<std::string>& extensions);
    void setAllowFileUpload(bool allow);
    void setUploadLocation(const std::string& location);

    std::string getRoutePath() const;
    std::set<std::string> getMethods() const;
    std::string getRedirect() const;
    std::string getRootDirectoryPath() const;
    bool getDirectoryListing() const;
    std::string getDefaultFile() const;
    std::vector<std::string> getCgiExtensions() const;
    bool getAllowFileUpload() const;
    std::string getUploadLocation() const;

private:
    std::string routePath;
    std::set<std::string> methods;
    std::string redirect;
    std::string rootDirectoryPath;
    bool directoryListing;
    std::string defaultFile;
    std::vector<std::string> cgiExtensions;
    bool allowFileUpload;
    std::string uploadLocation;
};

#endif
