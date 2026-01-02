#include <string>
#include <iostream>
#include <map>

struct Request
{
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

static int parse_header(std::string in, Request& req)
{
    size_t  headers_start = in.find("\r\n") + 2;
    size_t  header_end_marker = in.find("\r\n\r\n");

    for (size_t i = headers_start; i < header_end_marker; ++i)
    {
        size_t  k = in.find("\r\n", i);
    }

    size_t pos_colon = after_first.find(": ");
    std::string key = after_first.substr(0, pos_colon);
    size_t  pos_newline = after_first.find("\r\n");
    std::string value = after_first.substr(pos_colon + 2, pos_newline - key.size() - 2);
    std::cout << "key: [" << key << "]" << std::endl;
    std::cout <<"value: [" << value << "]" << std::endl;
    size_t  pos = in.find("\r\n");
    if (pos == std::string::npos)
        return -1;
    std::string str = in.substr(pos + 2);
    while (str.find("\r\n"))
    {
        size_t pos_colon = str.find(": ");
        std::string key = str.substr(0, pos_colon);
        size_t  pos_newline = str.find("\r\n");
        std::string value = str.substr(pos_colon + 2, pos_newline - key.size() - 2);
        req.headers.insert(std::make_pair(key, value));
        str = str.substr(pos + 2);
    }
    return 1;

}

int main () {
    std::string str = "GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n";
    Request req;
    parse_header(str, req);
}