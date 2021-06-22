#ifndef PARSER_H
#define PARSER_H

//Parser interface for parsing HTTP request lines

#include "Http.h"
#include <string>
#include <regex>
#include <vector>
#include <unordered_map>

namespace
{
    //table for scalable mapping of methods and headers
    const std::unordered_map<std::string, Http::Method> methodParseMap{{"GET", Http::Method::GET},
                                                                       {"HEAD", Http::Method::HEAD}};

    const std::unordered_map<std::string, Http::Header> headerParseMap{{"connection", Http::Header::Connection},
                                                                       {"content-type", Http::Header::ContentType},
                                                                       {"content-length", Http::Header::ContentLength},
                                                                       {"server", Http::Header::Server}};

    //regexes to validate line
    const std::regex startLineReg("\\S+ [^ ]+ HTTP/1\\.1\\r");
    const std::regex fileReg("/[a-zA-Z0-9\\.\\/\\-]*");
    const std::regex headerReg("\\S+: *.+ *\r");
}

class Parser
{
public:
    //enum of currently parsed section
    enum class Section
    {
        startSection,
        headerSection,
        bodySection
    };

private:
    Section section; //indicates which section is currently parsed

    //splits string along whitespaces
    std::vector<std::string> split(const std::string &str) noexcept;

    //parses first line
    int parseStart(Http::Request &request, const std::string &line) noexcept;

    //parses header lines
    int parseHeader(Http::Request &request, const std::string &line) noexcept;

public:
    //initializes object with section field to Section::startSection
    Parser() noexcept;

    //getter
    Section getSection() const noexcept;

    //parses a given line of request
    //returns 200 on success, 400 on regex failure and 501 on not supported failure
    int parseLine(Http::Request &request, const std::string &line) noexcept;

    //resets section field to Section::startSection to make reusable parser
    void reset() noexcept;
};

#endif
