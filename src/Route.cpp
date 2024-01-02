#include "Route.hpp"
#include "ParsingUtils.hpp"

Route::Route()
{
    this->getMethod = false;
    this->postMethod = false;
    this->deleteMethod = false;
    this->has_redirect = false;
    this->directoryListing = false;
    this->hasCGI = false;
    this->allowFileUpload = false;
    this->hasDefaultFile = false;
    this->hasMaxBodySize = false;
    this->hasRootDirectoryPath = false;
    this->maxBodySize = 1000000;
    std::string cwd = ParsingUtils::getCurrentWorkingDirectory();
    this->rootDirectoryPath = cwd + "/webserver/";
    this->uploadLocation = cwd + "/webserver/uploads/";
}

// Setters
void Route::setHasRootDirectoryPath(bool value)
{
    this->hasRootDirectoryPath = value;
}

void Route::setHasDefaultFile(bool value)
{
    this->hasDefaultFile = value;
}

void Route::setHasMaxBodySize(bool value)
{
  this->hasMaxBodySize = value;
}

void Route::setMaxBodySize(int size)
{
    this->maxBodySize = size;
}

void Route::setCGIPath(const std::string& path)
{
    this->cgi_path = path;
}

void Route::setRoutePath(const std::string& path)
{
    this->routePath = path;
}

void Route::setGetMethod(const bool value)
{
    this->getMethod = value;
}

void Route::setPostMethod(const bool value)
{
    this->postMethod = value;
}

void Route::setDeleteMethod(const bool value)
{
    this->deleteMethod = value;
}

void Route::setRedirectLocation(const std::string& redirect)
{
    this->redirect = redirect;
}

void Route::setRedirect(const bool value)
{
    this->has_redirect = value;
}

void Route::setRootDirectoryPath(const std::string& path)
{
    this->rootDirectoryPath = path;
}

void Route::setDirectoryListing(bool listing)
{
    this->directoryListing = listing;
}

void Route::setDefaultFile(const std::string& file)
{
    this->defaultFile = file;
}

void Route::setHasCGI(bool value)
{
    this->hasCGI = value;
}

void Route::setCgiExtensions(const std::vector<std::string>& extensions)
{
    this->cgiExtensions = extensions;
}

void Route::setAllowFileUpload(bool allow)
{
    this->allowFileUpload = allow;
}

void Route::setUploadLocation(const std::string& location)
{
    this->uploadLocation = location;
}

// getters
std::string Route::getRoutePath() const
{
    return this->routePath;
}

bool Route::getGetMethod(void) const
{
    return this->getMethod;
}

bool Route::getPostMethod(void) const
{
    return this->postMethod;
}

bool Route::getRedirect(void) const
{
    return this->has_redirect;
}

std::string Route::getRedirectLocation() const
{
    return this->redirect;
}

std::string Route::getRootDirectoryPath() const
{
    return this->rootDirectoryPath;
}

bool Route::getDirectoryListing() const
{
    return this->directoryListing;
}

std::string Route::getDefaultFile() const
{
    return this->defaultFile;
}

bool Route::getHasCGI() const
{
    return this->hasCGI;
}

std::vector<std::string> Route::getCgiExtensions() const
{
    return this->cgiExtensions;
}

bool Route::getAllowFileUpload() const
{
    return this->allowFileUpload;
}

std::string Route::getUploadLocation() const
{
    return this->uploadLocation;
}

bool Route::getHasDefaultFile() const
{
    return this->hasDefaultFile;
}

bool Route::getDeleteMethod() const
{
    return this->deleteMethod;
}

std::string Route::getCGIPath() const
{
    return this->cgi_path;
}

int Route::getMaxBodySize() const
{
    return this->maxBodySize;
}

bool Route::getHasMaxBodySize() const
{
  return this->hasMaxBodySize;
}

bool Route::getHasRootDirectoryPath() const
{
    return this->hasRootDirectoryPath;
}
