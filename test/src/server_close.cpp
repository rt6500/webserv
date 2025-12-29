#include "server.hpp"
#include <iostream>
#include <unistd.h> //close
#include <vector>

static void remove_client(std::vector<int>& clients, int fd)
{
    for (std::vector<int>::iterator it = clients.begin();
        it !=clients.end();++it)
        {
            if (*it == fd)
            {
                clients.erase(it);
                return ;
            }
        }
}

void    handle_close(int fd, ConnMap& conns, fd_set& master_read, 
    fd_set& master_write, std::vector<int>& clients)
{
    FD_CLR(fd, &master_read);
    FD_CLR(fd, &master_write);
    close(fd);
    conns.erase(fd);
    remove_client(clients, fd);
    std::cout << "closed fd=" << fd << "\n";
}
