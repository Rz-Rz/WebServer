#include "ErrorPageManager.hpp"
#include <string>
#include <cstring>
#include <sstream>

ErrorPageManager::ErrorPageManager(const std::string& defaultPagePath)
    : defaultErrorPagePath(defaultPagePath) {}

void ErrorPageManager::setErrorPage(int errorCode, const std::string& pagePath) {
    customErrorPages[errorCode] = pagePath;
}

std::string ErrorPageManager::getErrorPage(int errorCode) const {
    std::map<int, std::string>::const_iterator it = customErrorPages.find(errorCode);
    if (it != customErrorPages.end()) {
        return it->second;
    }
    return getDefaultErrorPage();
}

std::string ErrorPageManager::getFormattedErrorPage(int errorCode) const {
	std::string errorMessage;
	switch (errorCode) {
    case 400:
        errorMessage = "Bad Request. Error code: 400";
        break;
    case 401:
        errorMessage = "Unauthorized. Error code: 401";
        break;
    case 403:
        errorMessage = "Forbidden. Error code: 403";
        break;
    case 404:
        errorMessage = "Not Found. Error code: 404";
        break;
    case 405:
        errorMessage = "Method Not Allowed. Error code: 405";
        break;
    case 408:
        errorMessage = "Request Timeout. Error code: 408";
        break;
    case 500:
        errorMessage = "Internal Server Error. Error code: 500";
        break;
    case 501:
        errorMessage = "Not Implemented. Error code: 501";
        break;
    case 502:
        errorMessage = "Bad Gateway. Error code: 502";
        break;
    case 503:
        errorMessage = "Service Unavailable. Error code: 503";
        break;
    case 504:
        errorMessage = "Gateway Timeout. Error code: 504";
        break;
    default:
	{
		std::stringstream ss;
		ss << errorCode;
		errorMessage = "An error occurred. Error code: " + ss.str();
		break;
	}
}
	// Additional logic to customize the error message based on the error code
	std::string errorPageContent = getDefaultErrorPage(); // Retrieve the template

	// Replace the placeholder with the actual error message
	size_t placeholderPos = errorPageContent.find("[ERROR_MESSAGE]");
	if (placeholderPos != std::string::npos) {
		errorPageContent.replace(placeholderPos, std::string("[ERROR_MESSAGE]").length(), errorMessage);
	}

	return errorPageContent;
}

std::string ErrorPageManager::getDefaultErrorPage() const {
    return defaultErrorPagePath;
}

