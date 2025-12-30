#include "net_utils.hpp"
#include <fcntl.h>  //fcntl
#include <cstdio>   //perror
#include <stdlib.h>
// #include <algorithm>
#include <iostream>

bool set_nonblocking(int fd)
{
    // Get file status flags
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("F_GETFL");
        return false;
    }
    // Set file status flags
    int ret =fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (ret == -1)
    {
        perror("F_SETFL");
        return false;
    }
    return true;
}

bool extract_path(const std::string& line, std::string& path)
{
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
