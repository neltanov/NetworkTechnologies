#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <string>
#include <boost/asio.hpp>

using namespace boost::asio;

class Socks5Proxy {
public:
    Socks5Proxy(io_context& io, int proxy_port);

private:
    ip::tcp::acceptor client_acceptor;
    ip::tcp::socket client_socket;

    void acceptConnection();
    void handleHandshake();
    void handleRequest();
    void startDataTransfer(ip::tcp::socket& client_socket, std::shared_ptr<boost::asio::ip::tcp::socket> remote_socket);

    static const int max_length = 1024;
    char buffer_data[max_length];
};

#endif // PROXY_SERVER_H