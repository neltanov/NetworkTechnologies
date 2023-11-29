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
    : socket(std::make_shared<tcp::socket>(io)),
     buffer() {}

    std::shared_ptr<tcp::socket> getSocket() {
        return socket;
    }

    char* data() {
        return buffer;
    }

    void close() {
        if (socket && socket->is_open()) {
            boost::system::error_code ec;
            socket->shutdown(tcp::socket::shutdown_both, ec);
            socket->close(ec);
        }
    }

private:
    std::shared_ptr<tcp::socket> socket;
    char buffer[4096];
};

#endif // CONNECTION_H