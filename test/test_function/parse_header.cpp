#include <string>
#include <iostream>
#include <map>

#define NOT_READY -1

struct Request
{
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

static void trim_spaces(std::string& in)
{
    std::string::size_type i = 0;
    while (i < in.size() && std::isspace(static_cast<unsigned char>(in[i])))
        i++;
    std::string::size_type k = in.size();
    while (k > 0 && std::isspace(static_cast<unsigned char>(in[k - 1])))
        k--;
    if (i == k)
    {
        in.clear();
        return;
    }

    in = in.substr(i, k - i);
}

static int parse_header(const std::string& in, Request& req)
{
    std::string::size_type  pos = in.find("\r\n");
    std::string::size_type  header_end = in.find("\r\n\r\n");
    if (pos == std::string::npos || header_end == std::string::npos)
        return NOT_READY;
    std::string::size_type  headers_start = pos + 2;
    std::string::size_type i = headers_start;
    while (i < header_end)
    {
        std::string::size_type  k = in.find("\r\n", i);
        if (k == std::string::npos || k > header_end)
        {
            std::cerr << "Error: invalid request\n";
            return 1;
        }
        std::string line = in.substr(i, k - i);
        if (line.empty())
            break ;
        std::string::size_type  value_start = line.find(':');
        if (value_start == std::string::npos)
        {
            std::cerr << "Error: invalid request\n";
            return 1;
        }
        std::string value = line.substr(value_start + 1);
        std::string key = line.substr(0, value_start);
        trim_spaces(value);
        trim_spaces(key);
        if (key.empty())
            return 400;
        req.headers[key] = value;
        i = k + 2;
    }
    return 0;
}

int main () {
    std::string str = "GET / HTTP/1.1\r\n    Host:     127.0.0.1:8080   \r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";
    Request req;
    parse_header(str, req);
    std::map<std::string, std::string>::iterator    it;
    std::cout << "Map Contents:" << std::endl;
    for (it = req.headers.begin(); it != req.headers.end(); ++it)
        std::cout << "key: [" << it->first << "]" << " value: [" << it->second << "]" << std::endl;
    return 0;
}

// int main() {
//     std::string str = "   abc  \n";
//     trim_spaces_preceding(str);
//     std::cout << "[" << str << "]" << std::endl;
//     trim_spaces_following(str);
//     std::cout << "[" << str << "]" << std::endl;
// }
