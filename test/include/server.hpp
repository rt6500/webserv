#ifndef SERVER_HPP
# define SERVER_HPP
# include <string>
# include <cstddef>         //size_t
# include <sys/select.h>    //fd_set
# include <map>
# include <netinet/in.h>     //sockaddr
# include <vector>

struct  Connection
{
    std::string in_buf;
    std::string out_buf;
    std::size_t out_sent;
    Connection() : out_sent(0) {}
};

struct Request
{
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

typedef std::map<int, Connection>   ConnMap;

int    setup_listen_socket(int& listen_fd, sockaddr_in& addr,
    fd_set& master_read, fd_set& master_write, 
    int& fd_max);

void    accept_new_client(int listen_fd, fd_set& master_read, 
    int& fd_max, ConnMap& conns, std::vector<int>& clients);
void    handle_read(int fd, ConnMap& conns, fd_set& master_read, fd_set& master_write,
    std::vector<int>& clients);
void    handle_close(int fd, ConnMap& conns, fd_set& master_read, fd_set& master_write, 
    std::vector<int>& clients);
void    handle_write(int fd, ConnMap& conns, fd_set& master_read, fd_set& master_write,
    std::vector<int>& clients);

void    send_function(int client_fd, const char *data, std::size_t len, int flags);

// std::string parse_path(char* buffer);

#endif
