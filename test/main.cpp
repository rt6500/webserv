#include <sys/socket.h> // socket
#include <iostream>
#include <netinet/in.h> //sockaddr
#include <cstring>      //memset
#include <unistd.h>     //close
#include "mini_werbserv.hpp"
#include <cstdio>
#include <sys/select.h> //select, fd_set, FD_*

int main(){
    /* === socket creates endpoint===*/
    // AF_INET = IPv4
    // SOCK_STREAM= TCP
    //  protocol = 0
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << listen_fd << std::endl;
    if (listen_fd == -1)
    {
        std::cerr << "socket" << std::endl;
        return 1;
    }

    /* === bind : assigns addres===*/
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) == -1)
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
    
    fd_set  master;
    fd_set  read_fds;

    FD_ZERO(&master);
    FD_SET(listen_fd, &master);
    int fd_max = listen_fd;

    while (1) {
        read_fds = master;
        select(fd_max + 1, &read_fds, NULL, NULL, NULL);
        for (int i = 0; i <= fd_max; ++i)
        {
            if (FD_ISSET(i, &read_fds) == 1)
            {
                if (i == listen_fd)
                {
                    int client_fd = accept(listen_fd, NULL, NULL);
                    if (client_fd == -1)
                    {
                        perror("accept");
                        continue;
                    }
                    FD_SET(client_fd, &master);
                    if (listen_fd < client_fd)
                        fd_max = client_fd;
                    std::cout << "accepted fd=" << client_fd << std::endl;
                    close(client_fd);
                }
                else
                    std::cout << "client fd readable fd=X" << std::endl;
            }

        }
    }
    close(listen_fd);
    return 0;
}