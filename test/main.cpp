#include <sys/socket.h> // socket
#include <iostream>
#include <netinet/in.h> //sockaddr
#include <cstring>      //memset

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << fd << std::endl;
    if (fd == -1)
    {
        std::cerr << "socket" << std::endl;
        return 1;
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1)
    {
        std::cerr << "bind" << std::endl;
        return 1;
    }
    return 0;
}