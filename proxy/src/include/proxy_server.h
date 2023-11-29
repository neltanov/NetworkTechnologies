#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include "connection_pool.h"

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
    ConnectionPool connection_pool;

    static const int max_length = 4096;
    // char buffer_data[max_length];

    void acceptConnection();
    void handleGreeting(Connection& connection);
    void readAuthMethods(const error_code& ec, std::size_t length, Connection& connection);
    void sendServerChoice(const error_code& ec, std::size_t length, std::size_t count_of_methods, Connection& connection);
    void handleConnectionRequest(const error_code& ec, std::size_t length, Connection& connection);

    void readAddressType(const error_code& ec, std::size_t length, Connection& connection);
    void handleAddress(const error_code& ec, std::size_t length, Connection& connection);

    void readIPv4Address(const error_code& ec, std::size_t length, Connection& connection);

    void getDomainLength(const error_code& ec, std::size_t length, Connection& connection);
    void getDomainName(const error_code& ec, std::size_t length, Connection& connection);

    void sendServerResponse(const error_code& ec, Connection& connection, std::shared_ptr<tcp::socket> remote_socket);
    
    void startDataTransfer(const error_code& ec, std::size_t, Connection& connection, std::shared_ptr<tcp::socket> remote_socket);
    void sendDataToServer(const error_code& ec, std::size_t length, Connection& connection, std::shared_ptr<tcp::socket> remote_socket);
    void sendDataToClient(const error_code& ec, std::size_t length, Connection& connection, std::shared_ptr<tcp::socket> remote_socket);
};

#endif // PROXY_SERVER_H