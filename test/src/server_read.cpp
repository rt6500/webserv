#include "server.hpp"
#include "net_utils.hpp"
#include <iostream>
#include <sstream>
#include <cerrno>
#include <vector>

#define NOT_READY -1

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
    FD_SET(fd, &master_write);
    FD_CLR(fd, &master_read);
}

// only checks format
static int  parse_request_line(Connection& conn)
{
    size_t  pos_line = conn.in_buf.find("\r\n");
    if (pos_line == std::string::npos)
        return NOT_READY;
    std::string line = conn.in_buf.substr(0, pos_line);
    if (!extract_method_path_version(line, conn.req))
        return 400;
    return 0;
}

// only decides resorce existence
static int route(const std::string& path)
{
    if (path == "/")
        return 200;
    else
        return 404;
}

static int parse_header(Connection& conn)
{
    std::string in = conn.in_buf;
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
            return 400;
        }
        std::string line = in.substr(i, k - i);
        if (line.empty())
            break ;
        std::string::size_type  value_start = line.find(':');
        if (value_start == std::string::npos)
        {
            std::cerr << "Error: invalid request\n";
            return 400;
        }
        std::string value = line.substr(value_start + 1);
        std::string key = line.substr(0, value_start);
        trim_spaces(value);
        trim_spaces(key);
        if (key.empty())
            return 400;
        conn.req.headers[key] = value;
        i = k + 2;
    }
    return 0;
}

void    handle_read(int fd, ConnMap& conns, fd_set& master_read,
    fd_set& master_write, std::vector<int>& clients)
{
    ConnMap::iterator it = conns.find(fd);
    if (it == conns.end())
        return ;
    Connection& conn = it->second;

    char    buffer[1024];
    /*=== recv ===*/
    // received bytes recv(connected socket, buffer to read into, max bytes, flags)
    ssize_t size = recv(fd, buffer, sizeof(buffer), 0);
    if (size > 0)
    {
        std::cout << "data arrived fd=" << fd << ", bytes=" << size << "\n";
        conn.in_buf.append(buffer, size);
        if (conn.in_buf.size() > 8 * 1e3)
        {
            commit_response(conn, fd, master_write, master_read, 413);
            conn.in_buf.clear();
            return ;
        }

        int ret1 = parse_request_line(conn);
        if (ret1 == NOT_READY)
            return;
        if (ret1 == 400)
        {
            commit_response(conn, fd, master_write, master_read, 400);
            conn.in_buf.clear();
            return ;
        }
        int ret2 = parse_header(conn);
        if (ret2 == NOT_READY)
            return ;
        if (ret2 == 400)
        {
            commit_response(conn, fd, master_write, master_read, 400);
            conn.in_buf.clear();
            return ;
        }
        int code = route(conn.req.path);
        commit_response(conn, fd, master_write, master_read, code);
        std::cout << conn.out_buf << "\n";
        conn.in_buf.clear();
        // std::cout.write(buffer, size);
        return ;
    }
    // recv() == 0
    // peer closed connection (FIN)
    // server kernel marks the sockets as readable with EOF
    else if (size == 0)
        handle_close(fd, conns, master_read, master_write, clients);
    // recv() == -1
    // “No data was delivered to user space because the operation could not complete.”
    else
    {   //EINTR interrupted by a signal
        //EAGAIN/EWOULDBLOCK "nothing to read right now "
        // no close()!
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        handle_close(fd, conns, master_read, master_write, clients);
    }
}
