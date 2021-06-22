#include <filesystem>
#include <string>
#include "Server.h"

int main(int argc, char *argv[])
{
    //<nazwa-katalogu-z-plikami> <plik-z-serwerami-skorelowanymi> [<numer-portu-serwera>]
    if (argc < 3 || argc > 4)
        exit(EXIT_FAILURE);

    //portNum defaults to 8080
    int portNum = 8080;
    if (argc == 4)
        portNum = atoi(argv[3]);

    //working directory path save
    std::filesystem::path serverDirectory(argv[1]);
    if (!std::filesystem::exists(serverDirectory))
        exit(EXIT_FAILURE);

    //corelated files file open
    std::ifstream corelatedFile(argv[2]);
    if (!corelatedFile.good())
        exit(EXIT_FAILURE);

    //CORELATED FILE -> <ADDRESS, PORT> taken from file
    std::unordered_map<std::string, std::pair<std::string, int>> corelatedFilesMap;
    std::string fileName, ip;
    int serverPort;
    while (corelatedFile >> fileName)
    {
        corelatedFile >> ip >> serverPort;
        corelatedFilesMap.insert(std::pair<std::string, std::pair<std::string, int>>(fileName, std::pair<std::string, int>(ip, serverPort)));
    }

    //running server
    Server server(portNum, serverDirectory, corelatedFilesMap);
    server.run();
    return 0;
}
