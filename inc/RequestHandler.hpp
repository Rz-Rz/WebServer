#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "HTTPRequestParser.hpp"
#include "EventHandler.hpp"
#include "Server.hpp"
#include "MultipartFormDataParser.hpp"
#include "Reactor.hpp"
#include "Cookie.hpp"

class RequestHandler : public EventHandler {
  private: 
    HTTPRequestParser parser;
    Reactor* reactor;
    Cookie cookie;
    bool closeConnectionFlag;

    void handleGetRequest(const Server* server);
    void handlePostRequest(const Server* server);
    void handleDeleteRequest(const Server* server);
    void handleRedirect(const Route& route);
    void handleDirectoryRequest(const Route& route, const Server* server);
    void handleFileRequest(const Route& route, const Server* server);
    void handleFileUpload(const Route& route, const Server* server);
    void handleCGIRequest(const Route& route, const Server* server);
    void handleSession(void);

    bool isPayloadTooLarge(const Server* server, const Route& route);
    bool shouldCloseConnection(void);
    std::string generateDirectoryListingPage(const std::vector<std::string>& contents, const std::string& directoryPath);
    std::string extractFilename(const HTTPRequestParser& parser);
    std::string getFilename(const MultipartFormDataParser& parser);
    std::string removeFilename(const std::string& uri);
    std::string extractQueryString(const std::string& uri);
    std::string extractRouteFromUri(const std::string& uri);
    std::string removeQueryString(const std::string& uri);
    std::string endWithSlash(const std::string& uri);
    std::string extractSessionIdFromCookie(const std::string& cookie);
    Server* findServerForHost(const std::string& host, const std::map<std::string, Server*>* serversMap);

  public:
    RequestHandler();
    explicit RequestHandler(int fd, Reactor* reactor);
    virtual ~RequestHandler();

    void handleEvent(uint32_t events);
    void handleRequest(const Server* server);
    std::string getFilePathFromUri(const Route& route, const std::string& uri);
    std::string getUploadDirectoryFromUri(const Route& route, const std::string& uri);
    std::string extractDirectoryPath(const std::string& filePath);
    std::string getMimeType(const std::string& filePath);
    void closeConnection(void);
};
#endif
