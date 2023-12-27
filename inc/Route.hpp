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
    void setDeleteMethod(const bool value);
    void setRedirectLocation(const std::string& redirect);
    void setRedirect(const bool value);
    void setRootDirectoryPath(const std::string& path);
    void setDirectoryListing(bool listing);
    void setDefaultFile(const std::string& file);
    void setHasCGI(bool value);
    void setCgiExtensions(const std::vector<std::string>& extensions);
    void setAllowFileUpload(bool allow);
    void setUploadLocation(const std::string& location);
    void setHasDefaultFile(bool value);
    void setCGIPath(const std::string& path);
    void setMaxBodySize(int size);
    void setHasMaxBodySize(bool value);
    void setHasRootDirectoryPath(bool value);

    std::string getRoutePath() const;
    bool getGetMethod() const;
    bool getPostMethod() const;
    bool getDeleteMethod() const;
    std::string getRedirectLocation() const;
    bool getRedirect() const;
    std::string getRootDirectoryPath() const;
    bool getDirectoryListing() const;
    std::string getDefaultFile() const;
    bool getHasCGI() const;
    std::vector<std::string> getCgiExtensions() const;
    bool getAllowFileUpload() const;
    std::string getUploadLocation() const;
    bool getHasDefaultFile() const;
    std::string getCGIPath() const;
    int getMaxBodySize() const;
    bool getHasMaxBodySize() const;
    bool getHasRootDirectoryPath() const;

private:
    std::string routePath;
    bool getMethod;
    bool postMethod;
    bool deleteMethod;
    bool has_redirect;
    std::string redirect;
    std::string rootDirectoryPath;
    bool directoryListing;
    std::string defaultFile;
    bool hasDefaultFile;
    bool hasCGI;
    std::string cgi_path;
    std::vector<std::string> cgiExtensions;
    bool allowFileUpload;
    std::string uploadLocation;
    int maxBodySize;
    bool hasMaxBodySize;
    bool hasRootDirectoryPath;
};

#endif
