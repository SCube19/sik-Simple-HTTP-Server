#include "SocketTCP.h"
/////////////////////////////// CONSTRUCTOR /////////////////////////////////////////////////////
SocketTCP::SocketTCP(int portArg, int queueLengthArg) : port(portArg), queueLength(queueLengthArg), client_address_len(sizeof(sockaddr_in))
{
    //creating IPv4 TCP socket
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        exit(EXIT_FAILURE);

    //for later address reusal if we ever could terminate server normally
    int option = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
        exit(EXIT_FAILURE);

    server_address.sin_family = AF_INET;                // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(port);              // listening on port

    // bind the socket to a concrete address
    if (bind(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        exit(EXIT_FAILURE);

    // switch to listening (passive open)
    if (listen(sock, queueLength) < 0)
        exit(EXIT_FAILURE);
}

/////////////////// DESTRUCTOR /////////////////////////
SocketTCP::~SocketTCP()
{
    closeTCP();
}

/////////////////////////////// GETTERS /////////////////////////////////////////////////////
int SocketTCP::getSock() const noexcept
{
    return sock;
}

int SocketTCP::getMsgSock() const noexcept
{
    return msgFd;
}

int SocketTCP::getPort() const noexcept
{
    return port;
}

std::string SocketTCP::getMsg() const noexcept
{
    return msgString;
}

bool SocketTCP::clientDisconnected() const noexcept
{
    return socketContent->eof();
}

bool SocketTCP::readErr() const noexcept
{
    return socketContent->fail();
}

/////////////////////////////// ACCEPT TCP /////////////////////////////////////////////////////
void SocketTCP::acceptTCP()
{
    //accepting connetion with a client
    if ((msgFd = accept(sock, (struct sockaddr *)&client_address, &client_address_len)) < 0)
        exit(EXIT_FAILURE);

    //setting timeout to 10m
    timeval timeout;
    timeout.tv_sec = 600;
    timeout.tv_usec = 0;
    if (setsockopt(msgFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        exit(EXIT_FAILURE);

    //making istream pointing to communication descriptor
    filebuffIn = std::make_unique<__gnu_cxx::stdio_filebuf<char>>(msgFd, std::ios::in);
    socketContent = std::make_unique<std::istream>(&*filebuffIn);
}

/////////////////////////////// GETLINE /////////////////////////////////////////////////////
void SocketTCP::getline()
{
    std::getline(*socketContent, msgString);
}

void SocketTCP::getlineHTTP()
{
    std::string tmp = "";
    do
    {
        getline();
        tmp += msgString;
    } while (msgString.size() > 0 && msgString[msgString.size() - 1] != '\r');
    msgString = tmp;
}
/////////////////////////////// WRTIE TCP /////////////////////////////////////////////////////
void SocketTCP::writeTCP(const std::string &writeMsg) const
{
    //as max socket size is 2^15 sending it in chunks is necessary
    std::size_t writePtr = 0;
    while (writePtr < writeMsg.size())
    {
        std::size_t len = std::min((std::size_t)((1 << 15) - 1), writeMsg.size() - writePtr);
        if (write(msgFd, writeMsg.c_str() + writePtr, len) != (ssize_t)len)
        {
            //as second SIGPIPE for some reason didnt go inside signal handler
            //we just return if EPIPE is set
            if (errno == EPIPE)
            {
                errno = EIO;
                return;
            }
            exit(EXIT_FAILURE);
        }

        writePtr += len;
    }
}

/////////////////////////////// CLOSE MSG TCP /////////////////////////////////////////////////////
void SocketTCP::closeMsgTCP()
{
    if (filebuffIn->is_open() && filebuffIn->close() == NULL)
        exit(EXIT_FAILURE);
}

/////////////////////////////// CLOSE TCP /////////////////////////////////////////////////////
void SocketTCP::closeTCP()
{
    if (close(sock) < 0)
        exit(EXIT_FAILURE);
}
