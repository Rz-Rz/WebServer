#include "Route.hpp"

// Setters
void Route::setRoutePath(const std::string& path)
{
    this->routePath = path;
}

void Route::setMethods(const std::set<std::string>& methods)
{
    this->methods = methods;
}

void Route::setRedirect(const std::string& redirect)
{
    this->redirect = redirect;
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

std::set<std::string> Route::getMethods() const
{
    return this->methods;
}

std::string Route::getRedirect() const
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
