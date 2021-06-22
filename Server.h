
#ifndef SERVER_H
#define SERVER_H

//Server object to handle all connections

#include "SocketTCP.h"
#include "Parser.h"
#include "Http.h"
#include <filesystem>
//for SIGPIPE handling
#include <csignal>
#include <setjmp.h>

class Server
{
    int portNum;                                                                    //server port number
    std::filesystem::path serverDirectory;                                          //server working directory
    std::unordered_map<std::string, std::pair<std::string, int>> corelatedFilesMap; //corelated file -> <ip, port>

    //gets whole request from a socket, saves parser return code inside &code
    Http::Request getRequest(SocketTCP &socket, int &code) const;

public:
    //simple constructor
    Server(int portArg, const std::filesystem::path &dir, const std::unordered_map<std::string, std::pair<std::string, int>> filesMap) noexcept;

    //running a server
    void run() const;
};
#endif