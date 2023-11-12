#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using namespace boost::asio;

class Socks5Proxy {
public:
    Socks5Proxy(io_context& io, int proxy_port);

private:
    ip::tcp::acceptor client_acceptor;
    ip::tcp::socket client_socket;
    static const int max_length = 4096;
    char buffer_data[max_length];

    void acceptConnection();
    void handleGreeting();
    void readAuthMethods(const boost::system::error_code& ec, std::size_t length);
    void sendServerChoice(const boost::system::error_code& ec, std::size_t length, std::size_t count_of_methods);
    void handleConnectionRequest(const boost::system::error_code& ec, std::size_t length);
    void readAddressType(const boost::system::error_code& ec, std::size_t length);
    void handleAddress(const boost::system::error_code& ec, std::size_t length);
    void readIPv4Address(const boost::system::error_code& ec, std::size_t length);
    void sendServerResponse(const boost::system::error_code& ec, std::shared_ptr<boost::asio::ip::tcp::socket> remote_socket);
    void startDataTransfer(const boost::system::error_code& ec, std::size_t, std::shared_ptr<boost::asio::ip::tcp::socket> remote_socket);
    void sendDataToServer(const boost::system::error_code& ec, std::size_t length, std::shared_ptr<boost::asio::ip::tcp::socket> remote_socket);
    void sendDataToClient(const boost::system::error_code& ec, std::size_t length, std::shared_ptr<boost::asio::ip::tcp::socket> remote_socket);
};

#endif // PROXY_SERVER_H