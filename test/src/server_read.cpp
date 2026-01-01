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
        case 404: return "Not Found";
        case 413: return "Payload Too Large";
        case 414: return "URI Too Long";
        case 400: return "Bad Request";
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

static int  extract_first_line_and_decide_status(Connection& conn, int& code)
{
    size_t  pos_line = conn.in_buf.find("\r\n");
    if (pos_line == std::string::npos)
        return 1;
    std::string line = conn.in_buf.substr(0, pos_line);
    if (line.substr(0, 4) != "GET ")
        return 1;
    std::string path;
    if (!extract_path(line, path))
        code = 400;
    else
        code = decide_status(path);
    conn.out_buf = response(code);
    return 0;
}

static void response_413(Connection& conn, int fd, fd_set& master_write, fd_set& master_read)
{
    conn.out_buf = response(413);
    conn.out_sent = 0;
    FD_SET(fd, &master_write);
    FD_CLR(fd, &master_read);
    return ;
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
    ssize_t size = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (size > 0)
    {
        std::cout << "data arrived fd=" << fd << ", bytes=" << size << "\n";
        conn.in_buf.append(buffer, size);
        int code = 0;;
        if (conn.in_buf.size() > 8 * 1e3)
        {
            response_413(conn, fd, master_write, master_read);
            return ;
        }
        if (conn.in_buf.find("\r\n\r\n") == std::string::npos)
            return ;
        if (extract_first_line_and_decide_status(conn, code))
        {
            std::cerr << "invalid request\n";
            conn.out_buf =  response(400);
            conn.out_sent = 0;
            FD_SET(fd, &master_write);
            FD_CLR(fd, &master_read);
            return ;
        }
        //TODO remove processed bytes
        conn.out_sent = 0;
        FD_SET(fd, &master_write);
        FD_CLR(fd, &master_read);
        std::cout << conn.out_buf << "\n";
        // std::cout.write(buffer, size);
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
