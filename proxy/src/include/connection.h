#ifndef CONNECTION_H
#define CONNECTION_H

#include <array>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::tcp;

class Connection {
public:
    Connection(io_context& io) 
    : socket(io) {}

    tcp::socket& getSocket()  {
        return socket;
    }

    char* data() {
        return buffer;
    }

    char* recv_data() {
        return recv_buf;
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
    char buffer[8192];
    char recv_buf[8192];
};

#endif // CONNECTION_H