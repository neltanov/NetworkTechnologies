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
using namespace boost::placeholders;
using boost::asio::ip::tcp;

class FTPv2Server {
public:
    FTPv2Server(int server_port);
    void start();

private:
    void handleConnection(tcp::socket socket);
    void printSpeedInfo();
    size_t readHandler(const boost::system::error_code &ec, std::size_t bytes_transferred);
    
    boost::asio::io_context io_context;
    tcp::acceptor acceptor;
    std::map<ip::tcp::socket*, ClientInfo> clients;
    std::mutex clients_mutex;
    uint64_t file_size;
};

#endif // FTPV4_SERVER_H