#ifndef FTPV4_SERVER_H
#define FTPV4_SERVER_H

#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include "connection.h"

using namespace std;
using namespace boost::asio;
using namespace boost::placeholders;
using boost::asio::ip::tcp;

class FTPv4Server {
public:
    FTPv4Server(boost::asio::io_context& io_context,
                    unsigned short server_port);

private:
    void startAccept();
    void handleAccept(TCPConnection::pointer new_connection,
                      const boost::system::error_code& error);

    boost::asio::io_context& io_context;
    tcp::acceptor acceptor;
};

#endif // FTPV4_SERVER_H