#include "server.hpp"
#include "net_utils.hpp"
#include <netinet/in.h> //sockaddr
#include <iostream>
#include <cstring>      //memset
#include <unistd.h>     //close
#include <cstdio>   //perror

int    setup_listen_socket(int& listen_fd, sockaddr_in& addr,
    fd_set& master_read, fd_set& master_write, 
    int& fd_max)
{
    listen_fd =socket(AF_INET, SOCK_STREAM, 0);
    std::cout << listen_fd << "\n";
    if (listen_fd == -1)
    {
        std::cerr << "socket" << "\n";
        return 1;
    }

    /* === bind : assigns addres===*/
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int enable_reuse = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable_reuse, sizeof(enable_reuse)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR)");
        return 1;
    }
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &enable_reuse, sizeof(enable_reuse)) < 0)
    {
        perror("setsockopt(SO_REUSEPORT)");
        return 1;
    }
    socklen_t   len = sizeof(addr);
    if (bind(listen_fd, (sockaddr*)&addr, len) == -1)
    {
        perror("bind");
        return 1;
    }

    /* === listen makes it passive===*/
    if (listen(listen_fd, 10) == -1)
    {
        perror("listen");
        return 1;
    }
    FD_ZERO(&master_read);
    FD_ZERO(&master_write);
    FD_SET(listen_fd, &master_read);
    fd_max = listen_fd;
    if (!set_nonblocking(listen_fd))
    {
        close(listen_fd);
        return 1;
    }
    return 0;
}
