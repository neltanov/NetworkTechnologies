#ifndef CONNECTION_H
#define CONNECTION_H

#include <array>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::tcp;

#define BUF_SIZE 8192

class Connection {
public:
    Connection(io_context& io) 
    : socket(io) {}

    tcp::socket& getSocket()  {
        return socket;
    }

    char* toServerBuf() {
        return to_server_buf;
    }

    char* toClientBuf() {
        return to_client_buf;
    }

    void close() {
        boost::system::error_code ec;
        socket.shutdown(tcp::socket::shutdown_both, ec);
        if (socket.is_open()) {
            socket.close(ec);
        }
    }

private:
    tcp::socket socket;
    char to_server_buf[BUF_SIZE];
    char to_client_buf[BUF_SIZE];
};

#endif // CONNECTION_H