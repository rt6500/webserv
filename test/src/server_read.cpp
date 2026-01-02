#include "server.hpp"
#include "net_utils.hpp"
#include <iostream>
#include <sstream>
#include <cerrno>
#include <vector>

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
static int  parse_request_line(Connection& conn, Request& req)
{
    size_t  pos_line = conn.in_buf.find("\r\n");
    if (pos_line == std::string::npos)
        return -1;  // not ready
    std::string line = conn.in_buf.substr(0, pos_line);
    if (!extract_method_path_version(line, req))
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

static int parse_header(std::string in)
{
    size_t  pos = in.find("\r\n");
    if (pos == std::string::npos)
        return -1;
    std::string after_first = in.substr(pos + 2);
    size_t pos_colon = after_first.find(":");
    std::string key = after_first.substr(0, pos_colon - 1);
    size_t  pos_newline = after_first.find("\r\n");
    std::string value = after_first.substr(pos_colon + 1, pos_newline - key.size());
    std::cout << "key: [" << key << "]" << std::endl;
    std::cout <<"value: [" << value << "]" << std::endl;
    return 1;

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
        // "\r\n\r\n" is the end-of-HTTP-headers marker
        if (conn.in_buf.find("\r\n\r\n") == std::string::npos)
            return ;
        parse_header(conn.in_buf);
        Request req;
        int ret = parse_request_line(conn, req);
        if (ret == 400)
        {
            commit_response(conn, fd, master_write, master_read, 400);
            conn.in_buf.clear();
            return ;
        }
        else if (ret == -1)
            return ;
        int code = route(req.path);
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
