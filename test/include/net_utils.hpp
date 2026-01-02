//set_nonblocking/ errno helper

#ifndef NET_UTILS_HPP
#define NET_UTILS_HPP
#include <string>
#include "server.hpp"

bool    set_nonblocking(int fd);
bool    extract_method_path_version(const std::string& line, Request& req);
#endif
