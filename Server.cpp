#include "Server.h"
#include <iostream>

namespace
{
    jmp_buf acceptJmp;
}

//uzasadnione użycie goto/jmp http://www.csl.mtu.edu/cs4411.ck/www/NOTES/non-local-goto/sig-1.html
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void sigPipeHandle(int signum)
{
    longjmp(acceptJmp, 1);
}

//closes all sockets and frees adress
void exiter(int signum)
{
    exit(0);
}
#pragma GCC diagnostic pop

///////////////////////////////////////////// CONSTRUCTOR /////////////////////////////////////////////////////////////////////
Server::Server(int portArg, const std::filesystem::path &dir,
               const std::unordered_map<std::string, std::pair<std::string, int>> filesMap) noexcept : portNum(portArg),
                                                                                                       serverDirectory(dir),
                                                                                                       corelatedFilesMap(filesMap)
{
}

//----------------------------------------- GET REQUEST ---------------------------------------------------------------------//
Http::Request Server::getRequest(SocketTCP &serverSocket, int &code) const
{
    Parser parser;         //parser object to parse input lines
    Http::Request request; //request object to analyze
    do
    {
        //getting line from socket
        serverSocket.getlineHTTP();
        //any error quits loop
        if (serverSocket.clientDisconnected() || serverSocket.readErr())
            break;

        //parsing line
        int tmp = parser.parseLine(request, serverSocket.getMsg());

        //hierarchical 400 > 501, 404 > any
        code = tmp == 400 ? tmp : (code == 501 || code == 404 ? code : tmp);

    } while (code != 400 && serverSocket.getMsg()[0] != '\r');

    //additional codes to specify socket errors
    if (serverSocket.clientDisconnected())
        code = -1;
    else if (serverSocket.readErr())
        code = 500;
    return request;
}

///////////////////////////////////////////// RUN /////////////////////////////////////////////////////////////////////
void Server::run() const
{
    //socket of our server
    SocketTCP serverSocket(portNum);

    //signal handlers
    signal(SIGPIPE, sigPipeHandle);
    signal(SIGTSTP, exiter);
    signal(SIGINT, exiter);

    //server loop
    for (;;)
    {
        setjmp(acceptJmp);
        //accepting connection from client
        serverSocket.acceptTCP();
        //we will send response after every request
        Http::Request request;
        Http::Response response;
        int code = 200;
        do
        {
            //getting request
            request = getRequest(serverSocket, code);
            if (code == -1)
                break;

            //400 or 500 close client connection (as 500 can really mess up parsing from later requests with the same client)
            if (code == 400)
                response = Http::Response(code, "", "", true);
            else if (code == 500)
                //readErr() is also set on timeout
                response = Http::Response(code, "Timeout/Server_error", "", true);
            else
                //making response based on a request
                response = request.execute(serverDirectory, corelatedFilesMap);

            //sending response to a client
            serverSocket.writeTCP(response.execute());
        } while (!response.isConnectionClosed() || serverSocket.clientDisconnected());

        //closing connection
        serverSocket.closeMsgTCP();
    }

    serverSocket.closeTCP();
}
