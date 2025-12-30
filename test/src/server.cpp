#include "server.hpp"
#include "net_utils.hpp"
#include <iostream>
#include <netinet/in.h> //sockaddr
#include <cstring>      //memset
#include <unistd.h>     //close
#include <cstdio>       //perror

int    setup_listen_socket(int& listen_fd, sockaddr_in& addr,
    fd_set& master_read, fd_set& master_write, int& fd_max)
{
    /* === socket : create a socket (endpoint)===*/
    // domain: AF_INET = IPv4
    // type: SOCK_STREAM = TCP
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << listen_fd << "\n";
    if (listen_fd == -1)
    {
        perror("socket");
        return 1;
    }

    /* === bind : assigns address ===*/
    //TODO: memset is not allowed!
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
        // htons:host to network short (16-bit, endian conversion)
    addr.sin_port = htons(8080);
        // htonl:host to network long (32-bit, endian conversion)
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int enable_reuse = 1;
    // setsockopt: change an option on the kernel socket object that this fd refers to
    // before bind()
    // SO_REUSEADDR: "If the only conflict is TIME_WAIT from old connections, allow bind anyway."
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable_reuse, sizeof(enable_reuse)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR)");
        return 1;
    }
    // SO_REUSEPORT: "multiple sockets can bind to exactly the same (IP, port) at the same time."
    // if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &enable_reuse, sizeof(enable_reuse)) < 0)
    // {
    //     perror("setsockopt(SO_REUSEPORT)");
    //     return 1;
    // }
    socklen_t   len = sizeof(addr);
    if (bind(listen_fd, (sockaddr*)&addr, len) == -1)
    {
        perror("bind");
        return 1;
    }

    /* === listen : socket gets role as passive server ===*/
    // instructs the kernel to queue incoming connection requests
    // kernel starts to work independently (TCP handshake -> promotion (incoplete queue -> accept queue))
    // backlog: how many completed handshakes should the kernel hold in the accept queue 
    if (listen(listen_fd, 10) == -1)
    {
        perror("listen");
        return 1;
    }

    FD_ZERO(&master_read);
    FD_ZERO(&master_write);
    FD_SET(listen_fd, &master_read);
    fd_max = listen_fd;
    /*=== O_NONBLOCK is set ===*/
    if (!set_nonblocking(listen_fd))
    {
        close(listen_fd);
        return 1;
    }
    return 0;
}
