#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "HTTPRequestParser.hpp"
#include "EventHandler.hpp"
#include "Server.hpp"
#include "MultipartFormDataParser.hpp"

class RequestHandler : public EventHandler {
  private: 
    int client_fd;
    HTTPRequestParser parser;
    Server server;

    void sendRedirectResponse(const std::string& redirectLocation);
    void sendSuccessResponse(const std::string& statusCode, const std::string& contentType, const std::string& content);
    void handleGetRequest(const Server& server);
    void handlePostRequest(const Server& server);
    void handleDeleteRequest(const Server& server);
    void handleRedirect(const Route& route);
    void handleDirectoryRequest(const Route& route);
    void handleFileRequest(const Route& route);
    void handleFileUpload(const Route& route);
    void handleCGIRequest(const Route& route);
    void setCGIEnvironment(const std::string& queryString);
    std::string executeCGI(const std::string& filePath);
    bool isPayloadTooLarge(void);
    std::string generateDirectoryListingPage(const std::vector<std::string>& contents, const std::string& directoryPath);
    std::string extractFilename(const HTTPRequestParser& parser);
    std::string getFilename(const MultipartFormDataParser& parser);
    std::string extractDirectoryPath(const std::string& filePath);
    std::string extractQueryString(const std::string& uri);
    std::string extractRouteFromUri(const std::string& uri);
    std::string endWithSlash(const std::string& uri);



  public:
    explicit RequestHandler(int fd, Server& server);
    virtual ~RequestHandler();

    void handle_event(uint32_t events);
    void handleRequest(const Server& server);
    std::string getFilePathFromUri(const Route& route, const std::string& uri);
    std::string getUploadDirectoryFromUri(const Route& route, const std::string& uri);
    int get_handle() const;
    void sendErrorResponse(int errorCode);
};
#endif
