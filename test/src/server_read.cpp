#include "server.hpp"
#include "net_utils.hpp"
#include <iostream>
#include <sstream>
#include <cerrno>
#include <vector>

#define NOT_READY 0
#define BAD_REQUEST -1

static std::string  code_interpret(int code)
{
    switch (code) {
        case 200: return "OK";
        case 404: return "Not Found";           //"request is valid, resource not found"
        case 413: return "Payload Too Large";   // "request is valid, but too large"
        case 414: return "URI Too Long";
        case 400: return "Bad Request";         // "request is invalid"
        default: return "Unkown";
    }
}

static std::string   response(int code)
{
    std::string body = "hello\n";
    std::ostringstream  oss;
    oss << "HTTP/1.1 " << code << " " << code_interpret(code) << "\r\n"
        << "Content-length: " << body.size() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    return oss.str();
}

static void commit_response(Connection& conn, int fd, fd_set& master_write, fd_set& master_read, int code)
{
    conn.out_buf = response(code);
    conn.out_sent = 0;
    conn.close_after_write = true;
    FD_SET(fd, &master_write);
    FD_CLR(fd, &master_read);
}

// only decides resorce existence
static int route(const std::string& path)
{
    if (path == "/")
        return 200;
    else
        return 404;
}

// only checks format
static ssize_t  parse_request_line(const std::string& in, Request& req)
{
    size_t  pos_line = in.find("\r\n");
    if (pos_line == std::string::npos)
        return NOT_READY;
    std::string line = in.substr(0, pos_line);
    if (!extract_method_path_version(line, req))
        return BAD_REQUEST;
    return (ssize_t)(pos_line + 2);
}

static ssize_t parse_header(const std::string& in, Request& req)
{
    std::string::size_type header_end = in.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return NOT_READY;
    std::string::size_type  line_end = in.find("\r\n");
    if (line_end == std::string::npos || line_end > header_end)
        return BAD_REQUEST;
    std::string::size_type i = line_end + 2;
    while (i < header_end)
    {
        std::string::size_type  k = in.find("\r\n", i);
        if (k == std::string::npos || k > header_end)
            return BAD_REQUEST;
        std::string line = in.substr(i, k - i);
        if (line.empty())
            break ;
        std::string::size_type  value_start = line.find(':');
        if (value_start == std::string::npos)
            return BAD_REQUEST;
        std::string key = line.substr(0, value_start);
        std::string value = line.substr(value_start + 1);
        trim_spaces(key);
        trim_spaces(value);
        if (key.empty())
            return BAD_REQUEST;
        req.headers[key] = value;
        i = k + 2;
    }
    return (ssize_t)(header_end + 4);
}

ssize_t parse_http_request(const std::string& in_buf, Request& req)
{
    ssize_t rl = parse_request_line(in_buf, req);
    if (rl == NOT_READY || rl == BAD_REQUEST)
        return rl;
    ssize_t hd = parse_header(in_buf, req);
    if (hd == NOT_READY || hd == BAD_REQUEST)
        return hd;
    return hd;
}

static int recv_into_buffer(int fd, Connection& conn)
{
    char buffer[1024];
    ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
    if (n > 0)
    {
        conn.in_buf.append(buffer, (size_t)n);
        return (int)n; // bytes append;
    }
    if (n == 0)
        return 0;   // peer closed(FIN)
    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        return -2;// would block/interrupted(ignore)
    return -1;  // fetal error
}

static bool process_incomming(Connection& conn, int fd, fd_set& master_write, fd_set& master_read)
{
    if (conn.in_buf.size() > 8 * 1000)
    {
            commit_response(conn, fd, master_write, master_read, 413);
            conn.in_buf.clear();
            conn.req = Request();
            return true;
    }

    ssize_t consumed= parse_http_request(conn.in_buf, conn.req);
    std::cout << "parsed=" << consumed
              << " remain=" << conn.in_buf.size() - consumed << "\n";
    if (consumed == NOT_READY)
        return false;
    if (consumed == BAD_REQUEST)
    {
            commit_response(conn, fd, master_write, master_read, 400);
            size_t  he = conn.in_buf.find("\r\n\r\n");
            if (he != std::string::npos)
                conn.in_buf.erase(0, he + 4);
            else
                conn.in_buf.clear();
            conn.req = Request();
            return true;
    }
    //success
    conn.in_buf.erase(0, (size_t)consumed);
    int code = route(conn.req.path);
    commit_response(conn, fd, master_write, master_read, code);
    conn.req = Request();
    return true;
}

void    handle_read(int fd, ConnMap& conns, fd_set& master_read,
    fd_set& master_write, std::vector<int>& clients)
{
    ConnMap::iterator it = conns.find(fd);
    if (it == conns.end())
        return ;
    Connection& conn = it->second;

    int r = recv_into_buffer(fd, conn);
    if (r > 0)
    {
        (void)process_incomming(conn, fd, master_write, master_read);
        return ;
    }
    if ( r == -2)
        return ;
    if (r == 0 || r == -1)
        handle_close(fd, conns, master_read, master_write, clients);
    return ;
}
