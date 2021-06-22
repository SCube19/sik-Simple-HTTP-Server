#include "Http.h"
#include <iostream>

using namespace Http;
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% REQUEST %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//////////////////////////////////////////// CONSTRUCTORS /////////////////////////////////////////////////////////
Request::Request() noexcept : headerMap()
{
}
Request::Request(Method methodArg, const std::string &targetArg) noexcept : headerMap(), method(methodArg), target(targetArg)
{
}

//////////////////////////////////////////// SETTERS /////////////////////////////////////////////////////////
void Request::setMethod(Method method, const std::string &target) noexcept
{
    this->method = method;
    this->target = target;
}

//////////////////////////////////////////// GETTERS /////////////////////////////////////////////////////////
const std::unordered_map<Http::Header, std::string> &Request::getHeaderMap() const noexcept
{
    return headerMap;
}

Method Request::getMethod() const noexcept
{
    return method;
}

std::string Request::getTarget() const noexcept
{
    return target;
}

//////////////////////////////////////////// ADD HEADER /////////////////////////////////////////////////////////
void Request::addHeader(Header header, const std::string &fieldValue)
{
    headerMap.insert(std::pair<Header, std::string>(header, fieldValue));
}

//______________________________________________ EXECUTION SECTION _____________________________________________________//
////////////////////////////////////////// EXECUTE ////////////////////////////////////////////////////////////////////
Response Request::execute(const std::filesystem::path &serverDirectory, const std::unordered_map<std::string, std::pair<std::string, int>> &corelatedFilesMap) const
{
    switch (method)
    {
    case Method::GET:
        return get(serverDirectory, corelatedFilesMap);
    case Method::HEAD:
        return head(serverDirectory, corelatedFilesMap);
    case Method::UNSUPPORTED:
    default:
        int execution = executeHeaders();
        return execution == 0       ? Response(501, "", true)
               : (execution == 400) ? Response(400, "", true)
                                    : Response(501, "", false);
    }
}

//----------------------------------- EXECUTE HEADERS ------------------------------------------------//
//a little bit stupid but enough for now
int Request::executeHeaders() const
{
    //assume all is good
    int rVal = 200;
    //iterate through headers
    for (const auto &[header, value] : headerMap)
    {
        switch (header)
        {
        //if client wants to disconnect
        case Connection:
            if (value == "close")
                rVal = 0;
            break;

        //check for body
        case ContentLength:
            //anything but 0* isn't acceptable
            try
            {
                if (std::stoi(value) != 0)
                    throw;
            }
            catch (const std::exception &e)
            {
                return 400;
            }

        default:
            break;
        }
    }
    return rVal;
}

//_____________________________________________ METHODS ________________________________________________________//
//----------------------------------------- GET ---------------------------------------------------------------//
Response Request::get(const std::filesystem::path &serverDirectory, const std::unordered_map<std::string, std::pair<std::string, int>> &corelatedFilesMap) const
{
    //first check headers for code 400
    int execution = executeHeaders();
    if (execution == 400)
        return Response(400, "", true);

    std::string fileContent;
    //1. check is target file exists
    //2. if yes then check whether absolute path of server working directory is prefix of absolute path of target file
    //tl;dr check if target file is inside our working directory or its subdirectories
    try
    {
        if (std::filesystem::exists(std::string(serverDirectory) + target) &&
        std::string(std::filesystem::canonical(std::string(serverDirectory) + target)).find(std::filesystem::canonical(serverDirectory)) != 0)
            return Response(404, "", execution == 0);

        //open target
        std::ifstream requestedFile(std::string(serverDirectory) + target);

        //if not found try in corelated files
        if (requestedFile.fail())
            return fromCorelated(corelatedFilesMap, execution);
        else
            fileContent = std::string((std::istreambuf_iterator<char>(requestedFile)), std::istreambuf_iterator<char>());
    }
    catch (const std::exception &e)
    {
        //should client try to open a directory we will end up here
        return Response(404, "", execution == 0);
    }

    //send response with wanted file
    return Response(200, fileContent, execution == 0);
}

//-------------------------------------------- FROM CORELATED -------------------------------------------------------------------//
Response Request::fromCorelated(const std::unordered_map<std::string, std::pair<std::string, int>> &corelatedFilesMap, int execution) const
{
    auto it = corelatedFilesMap.find(target);
    if (it != corelatedFilesMap.end())
    {
        //sending location in body for convience
        Response res(302, "http://" + it->second.first + ":" + std::to_string(it->second.second) + it->first, execution == 0);
        res.setLength(0);
        return res;
    }

    return Response(404, "", execution == 0);
}

//-----------------------------------------HEAD---------------------------------------------------------------//
Response Request::head(const std::filesystem::path &serverDirectory, const std::unordered_map<std::string, std::pair<std::string, int>> &corelatedFilesMap) const
{
    Response res = get(serverDirectory, corelatedFilesMap);
    if(res.getCode() == 302)
    {
        res.setLength(0);
        return res;
    }
    Response res2(res.getCode(), "", res.isConnectionClosed());
    res.setLength(res.getBody().size());
    res2.setLength(res.getBody().size());
    return res2;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% RESPONSE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//////////////////////////////////////// CONSTRUCTORS ////////////////////////////////////////////////
Response::Response() noexcept
{
}
Response::Response(int codeArg, const std::string &bodyArg, bool closeArg) noexcept : code(codeArg), body(bodyArg), closeConnection(closeArg), length(bodyArg.size())
{
    switch (codeArg)
    {
    case 200:
        reason = "OK";
        break;
    case 302:
        reason = "Found";
        break;
    case 400:
        reason = "Bad_Request";
        break;
    case 404:
        reason = "Not_Found";
        break;
    case 500:
        reason = "Internal_Server_Error";
        break;
    case 501:
        reason = "Not_Implemented";
        break;
    default:
        reason = "";
    }
}
Response::Response(int codeArg, const std::string &reasonArg, const std::string &bodyArg, bool closeArg) noexcept : code(codeArg),
                                                                                                                    body(bodyArg),
                                                                                                                    reason(reasonArg),
                                                                                                                    closeConnection(closeArg),
                                                                                                                    length(bodyArg.size())
                                                                                                
{
}

/////////////////////////////////////// GETTERS /////////////////////////////////////////////////////////////////
int Response::getCode() const noexcept
{
    return code;
}
std::string Response::getReason() const noexcept
{
    return reason;
}

std::string Response::getBody() const noexcept
{
    return body;
}

bool Response::isConnectionClosed() const noexcept
{
    return closeConnection;
}

/////////////////////////////////////////////////// SETTERS /////////////////////////////////////////////////////
void Response::setLength(int length) noexcept
{
    this->length = length;
}

/////////////////////////////////////// EXECUTE ///////////////////////////////////////////////////////////////////
std::string Response::execute() const
{
    //HTTP/1.1 CODE REASON \r\n
    //Server: kk418331_http_server\r\n
    //Content-Type: application/octet-stream\r\n
    //Content-Length: length
    //[closeConnection ? Connection:close]\r\n
    //[code == 302 ? Location: BODY]\r\n
    //\r\n
    //BODY

    std::string returnString("HTTP/1.1 " + std::to_string(code) + " " + reason + "\r\n");
    returnString += "Server: kk418331_http_server\r\n";
    returnString += "Content-Type: application/octet-stream\r\n";
    returnString += "Content-Length: " + std::to_string(length) + "\r\n";
    if (closeConnection)
        returnString += "Connection: close\r\n";
    if (code == 302)
        returnString += "Location: " + body + "\r\n\r\n";
    else
        returnString += "\r\n" + body;

    return returnString;
}