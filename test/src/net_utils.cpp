#include "net_utils.hpp"
#include <fcntl.h>  //fcntl
#include <cstdio>   //perror

bool set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("F_GETFL");
        return false;
    }
    int ret =fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (ret == -1)
    {
        perror("F_SETFL");
        return false;
    }
    return true;
}
