#pragma once

#include <string>

namespace comicsdb
{

struct Comic
{
    enum
    {
        DELETED_ISSUE = -1
    };
    std::string title;
    int issue{DELETED_ISSUE};
    std::string writer;
    std::string penciler;
    std::string inker;
    std::string letterer;
    std::string colorist;
};

std::string toJson(const Comic &comic);
Comic fromJson(const std::string &json);

} // namespace comicsdb
