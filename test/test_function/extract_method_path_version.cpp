
#include <string>
#include <iostream>
#include <cstdlib>
#include <map>

struct Request
{
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

bool extract_method_path_version(const std::string& line, Request& req)
{
    if (line.size() < 4)
        return false;
    size_t  pos = line.find(' ');
    size_t  pos2 = line.find(' ', pos + 1);
    if ( pos == std::string::npos || pos2 == std::string::npos)
        return false;
    req.method = line.substr(0, pos);
    req.path = line.substr(pos + 1, pos2 - pos - 1);
    req.version = line.substr(pos2 + 1);
    std::cout << "method: [" << req.method << "]" << std::endl;
    std::cout << "path: [" << req.path << "]" << std::endl;
    std::cout << "version: [" << req.version << "]" << std::endl;
    return true;
}

int main () {
    std::string input ="GET / HTTP/1.1";
    // "GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\
    // User-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";
    Request req;
    extract_method_path_version(input, req);
    return 0;
}