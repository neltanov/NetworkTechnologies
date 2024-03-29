#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include "connection_pool.h"
#include "proxy_const.h"

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
    std::map<std::string, ip::address_v4> resolved_hosts;

    static const int max_length = 8192;

    void acceptConnection();
    void handleGreeting(std::shared_ptr<Connection> connection);
    void readAuthMethods(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);
    void sendServerChoice(const error_code& ec, std::size_t length, std::size_t count_of_methods, std::shared_ptr<Connection> connection);
    void handleConnectionRequest(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);

    void readAddressType(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);
    void handleAddress(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);

    void readIPv4Address(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);
    void getDomainLength(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);
    void getDomainName(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);

    unsigned char getConnectionError(const error_code& ec);
    void sendServerResponse(const error_code& ec, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket, tcp::endpoint remote_endpoint);
    
    void startDataTransfer(const error_code& ec, std::size_t, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket);
    void sendDataToServer(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket);
    void sendDataToClient(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket);
    void recvDataFromClient(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket);
    void recvDataFromServer(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket);
};

#endif // PROXY_SERVER_H