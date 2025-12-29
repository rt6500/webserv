#include "server.hpp"
#include <iostream>
#include <sstream>
#include <cerrno>
#include <vector>

static std::string   response()
{
    std::string body = "hello\n";
    std::ostringstream  oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-length: " << body.size() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    return oss.str();
}

void    handle_read(int fd, ConnMap& conns, fd_set& master_read,
    fd_set& master_write, std::vector<int>& clients)
{
    ConnMap::iterator it = conns.find(fd);
    if (it == conns.end())
        return ;
    Connection& conn = it->second;

    char    buffer[1024];
    ssize_t size = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (size > 0)
    {
        std::cout << "data arrived fd=" << fd << ", bytes=" << size << "\n";
        buffer[size] = '\0';
        conn.in_buf.append(buffer, size);
        conn.out_buf = response();
        conn.out_sent = 0;
        FD_SET(fd, &master_write);
        FD_CLR(fd, &master_read);
        std::cout.write(buffer, size);
    }
    else if (size == 0) //EOF
        handle_close(fd, conns, master_read, master_write, clients);
    else
    {   //EINTR interrupted by a signal
        //EAGAIN/EWOULDBLOCK "nothing to read right now "
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        handle_close(fd, conns, master_read, master_write, clients);
    }
}
