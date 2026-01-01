
#include "server.hpp"
#include "net_utils.hpp"
#include <sys/select.h> //select
#include <cerrno>       //errno
#include <cstdio>       //perror
#include <iostream>
#include <vector>


int main(){

    int         listen_fd;
    sockaddr_in addr;

    //  interest sets (persistent) "Which fds do I WANT the kernel to watch"
    // change only when the program logic changes like after accept/close/send etc..
    fd_set      master_read;
    fd_set      master_write;
    //  working copies (temporary) "Which fds are READY THIS ITERATION?"
    //  overwritten every loop. destroyed by select()
    fd_set      read_fds;
    fd_set      write_fds;

    int         fd_max;
    ConnMap     conns;
    std::vector<int>    clients;

    if (setup_listen_socket(listen_fd, addr, master_read, master_write, fd_max))
        return 1;

    while (1) {
        read_fds = master_read;
        write_fds = master_write;
        /*=== select === */
        if (select(fd_max + 1, &read_fds, &write_fds, NULL, NULL) == -1)
        {
            if (errno == EINTR)
                continue;
            perror("select");
            continue;
        }
        /*=== accept ===*/
        //no queue in accept queue -> program is sleeping(blocking)
        //takes one waiting TCP connection from the listening socket’s queue
        //creates a new socket that represents that connection.
        if (FD_ISSET(listen_fd, &read_fds))
            accept_new_client(listen_fd, master_read, fd_max, conns, clients);
        for (size_t i = 0; i < clients.size(); )
        {
            int fd = clients[i];
            if (FD_ISSET(fd, &read_fds))
                handle_read(fd, conns, master_read, master_write, clients);
            if (FD_ISSET(fd, &write_fds))
                handle_write(fd, conns, master_read, master_write, clients);
            if (i < clients.size() && clients[i] == fd)
                ++i;
        }
    }
    return 0;
}
