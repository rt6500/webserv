#ifndef MINIWEBSERV_HPP
# define MINIWEBSERV_HPP
# include <cstdlib>
# include <string>
# include <netinet/in.h>


// int     handle_socket(int* listen_fd);
// int     handle_bind(sockaddr_in* addr, int* listen_fd);
// int     handle_listen(int* listen_fd);
void    send_function(int client_fd, char *buffer, size_t bytes, int flag);
// std::string parse_path(char* buffer);
bool    extract_path(std::string& request, std::string& path);
int     decide_status(std::string& path);

#endif