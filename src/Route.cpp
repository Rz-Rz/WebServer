#include "Route.hpp"


Route::Route()
{
    this->getMethod = false;
    this->postMethod = false;
    this->has_redirect = false;
    this->directoryListing = false;
    this->has_CGI = false;
    this->allowFileUpload = false;
}

// Setters
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
    this->has_CGI = value;
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
    return this->has_CGI;
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
