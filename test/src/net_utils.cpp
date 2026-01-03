#include "net_utils.hpp"
#include "server.hpp"
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

bool extract_method_path_version(const std::string& line, Request& req)
{
    if (line.size() < 4)
        return false;
    size_t  pos = line.find(' ');
    size_t  pos2 = line.find(' ', pos + 1);
    if ( pos == std::string::npos || pos2 == std::string::npos)
        return false;
    req.method = line.substr(0, pos);
    req.path = line.substr(pos + 1, pos2 - pos - 1);
    req.version = line.substr(pos2 + 1);
    std::cout << "method: [" << req.method << "]" << std::endl;
    std::cout << "path: [" << req.path << "]" << std::endl;
    std::cout << "version: [" << req.version << "]" << std::endl;
    return true;
}

void trim_spaces(std::string& in)
{
    std::string::size_type i = 0;
    while (i < in.size() && std::isspace(static_cast<unsigned char>(in[i])))
        i++;
    std::string::size_type k = in.size();
    while (k > 0 && std::isspace(static_cast<unsigned char>(in[k - 1])))
        k--;
    if (i == k)
    {
        in.clear();
        return;
    }

    in = in.substr(i, k - i);
}
