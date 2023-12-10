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

    static const int max_length = 4096;

    void acceptConnection();
    void handleGreeting(std::shared_ptr<Connection> connection);
    void readAuthMethods(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);
    void sendServerChoice(const error_code& ec, std::size_t length, std::size_t count_of_methods, std::shared_ptr<Connection> connection);
    void handleConnectionRequest(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);

    void readAddressType(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);
    void handleAddress(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);

    void readIPv4Address(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);
    void readIPv6Address(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);

    void getDomainLength(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);
    void getDomainName(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection);

    unsigned char getConnectionError(const error_code& ec);
    void sendServerResponse(const error_code& ec, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket, tcp::endpoint remote_endpoint);
    
    void startDataTransfer(const error_code& ec, std::size_t, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket);
    void readDataFromClient(const error_code& ec, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket);
    void readDataFromServer(const error_code& ec, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket);
    void writeDataToServer(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket, std::vector<char>& data);
    void writeDataToClient(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket, std::vector<char>& data);
};

#endif // PROXY_SERVER_H