#include <sys/socket.h> // socket
#include <iostream>
#include <netinet/in.h> //sockaddr
#include <cstring>      //memset
#include <unistd.h>     //close
#include <cstdio>
#include <sys/select.h>

int main(){
    /* === socket creates endpoint===*/
    // AF_INET = IPv4
    // SOCK_STREAM= TCP
    //  protocol = 0

    
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << listen_fd << "\n";
    if (listen_fd == -1)
    {
        std::cerr << "socket" << "\n";
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

    while (1) {
        /* === accept creates a connection ===*/
        int client_fd = accept(listen_fd, 0, 0);
        if (client_fd == -1)
        {
            perror("accept");
            continue;
        }
        /*=== recv ===*/
        char    buffer[1024];
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0)
        {
            std::cout << "success to read " <<  bytes << "bytes" << "\n";
            buffer[bytes] = '\0';

            /*=== parsing path ===*/
            std::string request = buffer;
            std::string path;
            if (!extract_path(request, path))
                std::cerr << "invalid request\n";
            else
                std::cout << "path: " << path << "\n";

            /*=== decide status ===*/
            int status = decide_status(path);
            std::cout << "status: " << status << "\n";
            /*=== send ===*/
            send_function(client_fd, buffer, bytes, 0);
        }
        else if (bytes == 0)
            std::cout << "client disconnected" << "\n";
        else
            perror("recv");

        close(client_fd);
    }
    close(listen_fd);
    return 0;
}