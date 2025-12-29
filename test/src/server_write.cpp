#include "server.hpp"
#include <sys/socket.h>
#include <cerrno>
#include <vector>
#include <iostream>

void handle_write(int fd, ConnMap& conns, fd_set& master_read,
    fd_set& master_write, std::vector<int>& clients)
{
    ConnMap::iterator   it = conns.find(fd);
    if (it == conns.end())
        return ;
    Connection& conn = it->second;
    size_t  rest = conn.out_buf.size() - conn.out_sent;
    std::cout << "out_buf.size(): " << conn.out_buf.size() << " out_sent: " << conn.out_sent << " rest: " << rest;
    ssize_t  sent_bytes = send(fd, conn.out_buf.data() + conn.out_sent, rest, 0);
    std::cout << " send: " << sent_bytes << "\n";
    if (sent_bytes > 0)
        conn.out_sent += sent_bytes;
    else if (sent_bytes == 0)
    {
        handle_close(fd, conns, master_read, master_write, clients);
        return ;
    }
    else
    {
        if (errno == EINTR ||errno == EAGAIN || errno == EWOULDBLOCK)
            return ;
        handle_close(fd, conns, master_read, master_write, clients);
        return ;
    }


    if (conn.out_sent == conn.out_buf.size())
        handle_close(fd, conns, master_read, master_write, clients);

}
