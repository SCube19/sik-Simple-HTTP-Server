#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

//Http request/response interface

#include <unordered_map>
#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace Http
{
    //supported http methods
    enum Method
    {
        GET,
        HEAD,
        UNSUPPORTED
    };

    //supported http headers
    enum Header
    {
        Connection,
        ContentType,
        ContentLength,
        Server
    };

    //====================================RESPONSE CLASS===================================================
    class Response
    {
        int code;             //response code
        std::string body;     //response body
        std::string reason;   //response reason for code
        bool closeConnection; //if connection:close is present in request - if we should close connection
        int length;     //for head response

    public:
        //default
        Response() noexcept;
        //uses default reason messages
        Response(int codeArg, const std::string &bodyArg, bool closeArg) noexcept;
        //uses custom reason messages
        Response(int codeArg, const std::string &reasonArg, const std::string &bodyArg, bool closeArg) noexcept;

        //getters
        int getCode() const noexcept;
        std::string getReason() const noexcept;
        std::string getBody() const noexcept;
        bool isConnectionClosed() const noexcept;

        //setters
        void setLength(int length) noexcept;
        //returns string http response based on attributes
        std::string execute() const;
    };

    //=========================================REQUEST CLASS=================================================
    class Request
    {
        std::unordered_map<Header, std::string> headerMap; //stores header-name : header-value pairs
        Method method;                                     //request's method
        std::string target;                                //method's target

        //GET
        Response get(const std::filesystem::path &serverDirectory, const std::unordered_map<std::string, std::pair<std::string, int>> &corelatedFilesMap) const;
        //HEAD
        Response head(const std::filesystem::path &serverDirectory, const std::unordered_map<std::string, std::pair<std::string, int>> &corelatedFilesMap) const;

        //search for file in corelated
        Response fromCorelated(const std::unordered_map<std::string, std::pair<std::string, int>> &corelatedFilesMap, int execution) const;

        //acknowlegde all headers with appropriate action
        int executeHeaders() const;

    public:
        //constructors
        Request() noexcept;
        Request(Method methodArg, const std::string &targetArg) noexcept;

        //setters
        void setMethod(Method method, const std::string &target) noexcept;

        //getters
        const std::unordered_map<Header, std::string> &getHeaderMap() const noexcept;
        Method getMethod() const noexcept;
        std::string getTarget() const noexcept;

        //adds new header to header map
        void addHeader(Header header, const std::string &fieldValue);

        //makes response based on request
        Response execute(const std::filesystem::path &serverDirectory, const std::unordered_map<std::string, std::pair<std::string, int>> &corelatedFilesMap) const;
    };
}
#endif