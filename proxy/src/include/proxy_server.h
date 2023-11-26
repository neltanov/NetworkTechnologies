#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include "socket_pool.h"

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::tcp;

class Socks5Proxy {
public:
    Socks5Proxy(unsigned short proxy_port);
    void start();

private:
    io_context io_ctx;
    tcp::acceptor client_acceptor;
    // tcp::socket client_socket;
    SocketPool socket_pool;

    static const int max_length = 4096;
    char buffer_data[max_length];

    void acceptConnection();
    void handleGreeting(std::shared_ptr<tcp::socket> socket_ptr);
    void readAuthMethods(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr);
    void sendServerChoice(const error_code& ec, std::size_t length, std::size_t count_of_methods, std::shared_ptr<tcp::socket> client_socket_ptr);
    void handleConnectionRequest(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr);
    void readAddressType(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr);
    void handleAddress(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr);
    void readIPv4Address(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr);
    void sendServerResponse(const error_code& ec, std::shared_ptr<tcp::socket> remote_socket, std::shared_ptr<tcp::socket> client_socket_ptr);
    void startDataTransfer(const error_code& ec, std::size_t, std::shared_ptr<tcp::socket> client_socket_ptr, std::shared_ptr<tcp::socket> remote_socket);
    void sendDataToServer(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr, std::shared_ptr<tcp::socket> remote_socket);
    void sendDataToClient(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr, std::shared_ptr<tcp::socket> remote_socket);
};

#endif // PROXY_SERVER_H