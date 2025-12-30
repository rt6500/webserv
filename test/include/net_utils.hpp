//set_nonblocking/ errno helper

#ifndef NET_UTILS_HPP
#define NET_UTILS_HPP
#include <string>


bool    set_nonblocking(int fd);
bool    extract_path(const std::string& request, std::string& path);
int     decide_status(const std::string& path);

#endif
