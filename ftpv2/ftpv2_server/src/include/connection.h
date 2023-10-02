#ifndef CONNECTION_H
#define CONNECTION_H

#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::placeholders;
using boost::asio::ip::tcp;

class TCPConnection : public boost::enable_shared_from_this<TCPConnection> {
public:
    typedef boost::shared_ptr<TCPConnection> pointer;

    static pointer create(boost::asio::io_context& io_context);

    tcp::socket& socket();

    void start();

private:
    TCPConnection(io_context& io_context);
    void handle_write(/* const boost::system::error_code& e, size_t bytes_transferred */);

    tcp::socket socket_;
    std::string message; /* TODO: поменять на класс Message */
};

#endif // CONNECTION_H