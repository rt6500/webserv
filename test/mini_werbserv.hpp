#ifndef MINIWEBSERV_HPP
# define MINIWEBSERV_HPP
# include <cstdlib>

void    send_function(int client_fd, char *buffer, size_t bytes, int flag);
std::string parse_path(char* buffer);

#endif