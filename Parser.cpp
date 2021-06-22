#include "Parser.h"
#include <iostream>
////////////////////////////////// CONSTRUCTOR //////////////////////////////////////////
Parser::Parser() noexcept : section(Section::startSection)
{
}

////////////////////////////////// GETTERS //////////////////////////////////////////
Parser::Section Parser::getSection() const noexcept
{
    return section;
}

//------------------------------- SPLIT -------------------------------------------//
std::vector<std::string> Parser::split(const std::string &str) noexcept
{
    std::istringstream iss(str);
    return std::vector<std::string>((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
}

//------------------------------- PARSE START -------------------------------------------//
int Parser::parseStart(Http::Request &request, const std::string &line) noexcept
{
    //after that we should parse headers
    section = Section::headerSection;
    //checks for [method SP request-target SP HTTP-version CRLF]
    if (!std::regex_match(line, startLineReg))
        return 400;

    std::vector<std::string> words = split(line);
    //checks if filename contains only valid characters
    if (!std::regex_match(words[1], fileReg))
        return 404;

    //find method inside supported method map
    auto it = methodParseMap.find(words[0]);
    if (it != methodParseMap.end())
    {
        request.setMethod(it->second, words[1]);
        return 200;
    }
    request.setMethod(Http::Method::UNSUPPORTED, words[1]);
    return 501;
}

//------------------------------- PARSE HEADER -------------------------------------------//
int Parser::parseHeader(Http::Request &request, const std::string &line) noexcept
{
    //empty line indicates the end of headers
    if (line[0] == '\r')
    {
        section = Section::bodySection;
        return 200;
    }
    //checks for [field-name ":" OWS field-value OWS]
    if (!std::regex_match(line, headerReg))
        return 400;

    //split into header, rest
    size_t terminator = line.find(":");
    std::string header(line.substr(0, terminator));
    std::string value(line.substr(terminator + 1));

    //trim rest to just header value
    size_t first = value.find_first_not_of(" ");
    size_t last = value.find_last_not_of(" \r");
    value = value.substr(first, last - first + 1);
    //header is case-insensitive
    std::transform(header.begin(), header.end(), header.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    //find and add header inside supported header map to request header map
    auto it = headerParseMap.find(header);
    //multiple occurences of supported header are not allowed
    if (it != headerParseMap.end() && request.getHeaderMap().find(it->second) != request.getHeaderMap().end())
        return 400;
    //supported header
    else if (it != headerParseMap.end())
        request.addHeader(it->second, value);
    return 200;
}

//////////////////////////////////PARSE LINE//////////////////////////////////////////
int Parser::parseLine(Http::Request &request, const std::string &line) noexcept
{
    switch (section)
    {
    case Section::startSection:
        return parseStart(request, line);
    case Section::headerSection:
        return parseHeader(request, line);
    default:
        return 400;
    }
}

////////////////////////////////// RESET //////////////////////////////////////////
void Parser::reset() noexcept
{
    section = Section::startSection;
}
