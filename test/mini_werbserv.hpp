#ifndef MINIWEBSERV_HPP
# define MINIWEBSERV_HPP
# include <cstdlib>
# include <string>

void    send_function(int client_fd, char *buffer, size_t bytes, int flag);
// std::string parse_path(char* buffer);
void    extract_path(std::string& request, std::string& path);
int     decide_status(std::string& path);
#endif