#include "server.hpp"
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <iostream>

bool extract_path(const std::string& request, std::string& path)
{
    size_t  pos_line = request.find("\r\n");
    if (pos_line == std::string::npos)
        return false;
    std::string line = request.substr(0, pos_line);
    size_t  pos = line.find(' ');
    size_t  pos2 = line.find(' ', pos + 1);
    if ( pos == std::string::npos || pos2 == std::string::npos)
        return false;
    path = line.substr(pos + 1, pos2 - pos - 1);
    return true;
}

int decide_status(const std::string& path)
{
    if (path == "/")
        return 200;
    else
        return 404;
}
