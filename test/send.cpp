#include "mini_werbserv.hpp"
#include <iostream>
#include <netinet/in.h> //send
#include <string>
#include <sstream>
#include <cstdio>

void    send_function(int client_fd, char *buffer, size_t bytes, int flag)
{
    (void)buffer;
    (void)bytes;
    (void)flag;
    std::string body = "hello\n";
    std::ostringstream  oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-length: " << body.size() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    std::string res = oss.str();
    ssize_t sent = send(client_fd, res.c_str(), res.size(), 0);
    if (sent == -1)
        perror("send");
    else
        std::cout << "success to send" << std::endl;
}