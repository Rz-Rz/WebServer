#include "RouteDebug.hpp"
#include <iostream>

void RouteDebug::printRouteInfo(const Route& route) {
  std::cout << "Route Path: " << route.getRoutePath() << std::endl;
  std::cout << "Get Method: " << std::boolalpha << route.getGetMethod() << std::endl;
  std::cout << "Post Method: " << std::boolalpha << route.getPostMethod() << std::endl;
  std::cout << "Redirect: " << std::boolalpha << route.getRedirect() << std::endl;
  std::cout << "Redirect Location: " << route.getRedirectLocation() << std::endl;
  std::cout << "Root Directory Path: " << route.getRootDirectoryPath() << std::endl;
  std::cout << "Directory Listing: " << std::boolalpha << route.getDirectoryListing() << std::endl;
  std::cout << "Default File: " << route.getDefaultFile() << std::endl;
  std::cout << "Has Default File: " << std::boolalpha << route.getHasDefaultFile() << std::endl;
  std::cout << "Has CGI: " << std::boolalpha << route.getHasCGI() << std::endl;
  std::cout << "CGI Extensions: ";
  std::vector<std::string> extensions = route.getCgiExtensions();
  for (std::size_t i = 0; i < extensions.size(); ++i) {
    std::cout << extensions[i];
    if (i != extensions.size() - 1) std::cout << ", ";
  }
  std::cout << std::endl;
  std::cout << "Allow File Upload: " << std::boolalpha << route.getAllowFileUpload() << std::endl;
  std::cout << "Upload Location: " << route.getUploadLocation() << std::endl;
}
