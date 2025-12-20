#include "mini_werbserv.hpp"
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <iostream>

void extract_path(std::string& request, std::string& path)
{
    size_t  pos_line = request.find("\r\n");
    if (pos_line == std::string::npos)
        std::cerr << "invalid format\n";
    std::string line = request.substr(0, pos_line);
    size_t  pos = line.find(' ');
    size_t  pos2 = line.find(' ', pos + 1);
    if ( pos == std::string::npos || pos2 == std::string::npos)
        std::cerr << "invalid format\n";
    path = line.substr(pos + 1, pos2 - pos - 1);
}

int decide_status(std::string& path)
{
    if (path == "/")
        return 200;
    else
        return 404;
}