#include "server.hpp"
#include "net_utils.hpp"
#include <netinet/in.h>    //accept
#include <unistd.h> //close
#include <iostream>
#include <sys/select.h> //FD_SET
#include <cerrno>   //errno
#include <cstdio>   //perror
#include <sys/socket.h> // recv/accept
#include <vector>

void    accept_new_client(int listen_fd, 
    fd_set& master_read, int& fd_max, ConnMap& conns, std::vector<int>& clients)
{
    while (1)
    {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd == -1)
        {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break ;
            perror("accept");
            break ;
        }
        if (!set_nonblocking(client_fd))
        {
            close(client_fd);
            continue;
        }
        Connection &conn = conns[client_fd];
        conn.out_sent = 0;

        FD_SET(client_fd, &master_read);
        if (client_fd > fd_max)
            fd_max = client_fd;
        clients.push_back(client_fd);
        std::cout << "accepted fd=" << client_fd << "\n";
    }

}
