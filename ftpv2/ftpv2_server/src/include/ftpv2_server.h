#ifndef FTPV4_SERVER_H
#define FTPV4_SERVER_H

#include <iostream>
#include <algorithm>
#include <fstream>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio/detail/socket_ops.hpp>

#include "client_info.h"

using namespace std;
using namespace boost::asio;
using namespace boost::filesystem;
using boost::asio::ip::tcp;

class FTPv2Server {
public:
    FTPv2Server(int server_port);
    void start();

private:
    void handleConnection(tcp::socket socket);
    size_t handleReading(boost::system::error_code& ec, size_t bytes_transferred);
    
    boost::asio::io_context io_context;
    tcp::acceptor acceptor;
    path uploads_dir;
};

#endif // FTPV4_SERVER_H