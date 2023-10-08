#ifndef FTPV4_SERVER_H
#define FTPV4_SERVER_H

#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
// #include <boost/thread.hpp>
// #include "connection.h"

using namespace std;
using namespace boost::asio;
using namespace boost::placeholders;
using boost::asio::ip::tcp;

class FTPv2Server {
public:
    FTPv2Server(boost::asio::io_context& io_context,
                    unsigned short server_port);

private:
    void startAccept();
    void handleConnection(tcp::socket socket);
    // void handleWrite(tcp::socket& socket, const boost::system::error_code& error);
    // void handleRead(tcp::socket& socket, const boost::system::error_code& error, size_t bytes_transferred);
    
    boost::asio::io_context& io_context;
    tcp::acceptor acceptor;
    // enum { max_buffer_size = 1024 };
    // char data_buffer[max_buffer_size];
};

#endif // FTPV4_SERVER_H