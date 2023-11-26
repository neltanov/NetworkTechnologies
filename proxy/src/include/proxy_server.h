#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

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
    tcp::socket client_socket;
    static const int max_length = 4096;
    char buffer_data[max_length];

    void acceptConnection();
    void handleGreeting();
    void readAuthMethods(const error_code& ec, std::size_t length);
    void sendServerChoice(const error_code& ec, std::size_t length, std::size_t count_of_methods);
    void handleConnectionRequest(const error_code& ec, std::size_t length);
    void readAddressType(const error_code& ec, std::size_t length);
    void handleAddress(const error_code& ec, std::size_t length);
    void readIPv4Address(const error_code& ec, std::size_t length);
    void sendServerResponse(const error_code& ec, std::shared_ptr<tcp::socket> remote_socket);
    void startDataTransfer(const error_code& ec, std::size_t, std::shared_ptr<tcp::socket> remote_socket);
    void sendDataToServer(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> remote_socket);
    void sendDataToClient(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> remote_socket);
};

#endif // PROXY_SERVER_H