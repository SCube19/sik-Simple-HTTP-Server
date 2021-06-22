#ifndef SOCKET_TCP_H
#define SOCKET_TCP_H

//Simple TCP socket interface for easy setup and write/read

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <memory>
#include <string>
#include <ext/stdio_filebuf.h>

class SocketTCP
{
    int port;              //connection port given by a user
    int sock;              //connection socket descriptor
    int msgFd;             //message exchange descriptor
    int queueLength;       //number of clients that can queue up to interact with server
    std::string msgString; //string to save read message

    sockaddr_in server_address;   //used by bind() function
    sockaddr_in client_address;   //used by accept() function
    socklen_t client_address_len; //

    //msgFd stream handling
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char>> filebuffIn;
    std::unique_ptr<std::istream> socketContent;

public:
    //sets socket, binds and switches object to passive listen mode
    SocketTCP(int portArg, int queueLengthArg = 127);
    ~SocketTCP();

    //simple getters
    int getSock() const noexcept;
    int getMsgSock() const noexcept;
    int getPort() const noexcept;
    std::string getMsg() const noexcept;
    bool clientDisconnected() const noexcept;
    bool readErr() const noexcept;

    //accept() and point object fields onto new message descriptor with timeout set to 30m
    void acceptTCP();

    //reads a line from a msgFd socket and saves in msgString
    void getline();
    //reads \r\n ended line()
    void getlineHTTP();

    //writes a writeMsg into a message descriptor via socket
    void writeTCP(const std::string &writeMsg) const;

    //closes message descriptor
    void closeMsgTCP();
    //closes socket descriptor and socketContent
    void closeTCP();
};
#endif