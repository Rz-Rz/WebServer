#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <map>
#include <string>
#include "Server.hpp"


class ServerManager {
public:
    static ServerManager& getInstance();
    void setServersMap(std::map<std::string, Server*>* map);
    std::map<std::string, Server*>* getServersMap() const;

private:
    std::map<std::string, Server*>* serversMap;

    ServerManager();
    ~ServerManager();
    // Disable Copy Constructor and Assignment
    ServerManager(const ServerManager&);
    ServerManager& operator=(const ServerManager&);
};

#endif
