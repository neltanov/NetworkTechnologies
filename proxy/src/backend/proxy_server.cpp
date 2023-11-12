#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "../include/proxy_server.h"

using namespace boost::asio;
using namespace boost::placeholders;

Socks5Proxy::Socks5Proxy(io_context& io, int proxy_port)
    : client_acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), proxy_port)),
        client_socket(io) {
    acceptConnection();
}


void Socks5Proxy::acceptConnection() {
    client_acceptor.async_accept(client_socket,
        [this](const boost::system::error_code& ec) {
            if (!ec) {
                std::cout << "New connection: " << client_socket.remote_endpoint() << std::endl;
                handleGreeting();
            }
            acceptConnection();
        });
}

void Socks5Proxy::handleGreeting() {
    // Reading client greeting (SOCKS version and number of authentication methods supported)
    async_read(client_socket, buffer(buffer_data, 2), boost::bind(&Socks5Proxy::readAuthMethods, this, _1, _2));
}

void Socks5Proxy::readAuthMethods(const boost::system::error_code& ec, std::size_t length) {
    if (!ec && length == 2 && buffer_data[0] == 5) {
        size_t count_of_methods = buffer_data[1];
        // Reading authentication methods
        async_read(client_socket, buffer(buffer_data, count_of_methods), 
                    boost::bind(&Socks5Proxy::sendServerChoice, this, _1, _2, count_of_methods));
    }
    else {
        std::cerr << "Auth methods receiving error: " << ec.message() << std::endl;
    }
}

void Socks5Proxy::sendServerChoice(const boost::system::error_code& ec, std::size_t length, std::size_t count_of_methods) {
    if (!ec && length == count_of_methods) {
        buffer_data[0] = 5;  // SOCKS5 version
        buffer_data[1] = 0;  // No authentification method
        // Sending server choice
        async_write(client_socket, buffer(buffer_data, 2), boost::bind(&Socks5Proxy::handleConnectionRequest, this, _1, _2));
    }
    else {
        std::cerr << "Server choice sending error: " << ec.message() << std::endl;
    }
}

void Socks5Proxy::handleConnectionRequest(const boost::system::error_code& ec, std::size_t) {
    if (!ec) {
        // Reading client connection request (Firstly, SOCKS version, command code and reserved byte)
        async_read(client_socket, buffer(buffer_data, 3), boost::bind(&Socks5Proxy::readAddressType, this, _1, _2));
    }
    else {
        std::cerr << "Client connection request is failed: " << ec.message() << std::endl;
    }
}

void Socks5Proxy::readAddressType(const boost::system::error_code& ec, std::size_t length) {
    if (!ec && length == 3 && buffer_data[0] == 5 && buffer_data[1] == 1 && buffer_data[2] == 0) {
        // Reading address type
        async_read(client_socket, buffer(buffer_data, 1), boost::bind(&Socks5Proxy::handleAddress, this, _1, _2));
    }
    else {
        std::cerr << "Address reading error: " << ec.message() << std::endl;
    }
}

void Socks5Proxy::handleAddress(const boost::system::error_code& ec, std::size_t length) {
    if (!ec && length == 1) {
        uint8_t address_type = buffer_data[0];
        if (address_type == 1) { // IPv4 address
            // Read 4 bytes address and 2 bytes port
            async_read(client_socket, buffer(buffer_data, 6), boost::bind(&Socks5Proxy::readIPv4Address, this, _1, _2));
        }
        else if (address_type == 3) { // TODO: DNS resolving
            std::cout << "DNS resolving is not done yet" << std::endl;

        }
        else if (address_type == 4) { // IPv6 address (don't need)
            std::cout << "IPv6 is not supported" << std::endl;
        }
    }
}

void Socks5Proxy::readIPv4Address(const boost::system::error_code& ec, std::size_t length) {
    if (!ec && length == 6) {
        boost::asio::ip::address_v4::bytes_type ipAddressBytes;
        memcpy(ipAddressBytes.data(), &buffer_data[0], 4);
        boost::asio::ip::address_v4 ip(ipAddressBytes);

        uint16_t port;
        memcpy(&port, &buffer_data[4], 2);
        port = ntohs(port);
        std::cout << "Remote server: " << ip.to_string() << ":" << port << std::endl;

        auto remote_socket = std::make_shared<ip::tcp::socket>(client_socket.get_executor());
        
        remote_socket->async_connect(ip::tcp::endpoint(ip, port), boost::bind(&Socks5Proxy::sendServerResponse, this, _1, remote_socket));
    }
}

void Socks5Proxy::sendServerResponse(const boost::system::error_code& ec, std::shared_ptr<boost::asio::ip::tcp::socket> remote_socket) {
    if (!ec) {
        std::cout << "Proxy connected to server " << remote_socket->remote_endpoint() << " successfully" << std::endl;
        // Response packet from server
        buffer_data[0] = 5;  // SOCKS version
        buffer_data[1] = 0;  // Success
        buffer_data[2] = 0;  // Reserved byte
        buffer_data[3] = 1;  // Address type (IPv4)
        async_write(client_socket, buffer(buffer_data, 10), 
                    boost::bind(&Socks5Proxy::startDataTransfer, this, _1, _2, remote_socket));
    }
    else {
        std::cerr << ec.message() << std::endl;
    }
}

void Socks5Proxy::startDataTransfer(const boost::system::error_code& ec, std::size_t, std::shared_ptr<boost::asio::ip::tcp::socket> remote_socket) {
    if (!ec) {
        std::cout << "Data transfer from client " << client_socket.remote_endpoint() << std::endl;
        client_socket.async_read_some(buffer(buffer_data, max_length), boost::bind(&Socks5Proxy::sendDataToServer, this, _1, _2, remote_socket));

        std::cout << "Data transfer from server " << remote_socket->remote_endpoint() << std::endl;
        remote_socket->async_read_some(buffer(buffer_data, max_length), boost::bind(&Socks5Proxy::sendDataToClient, this, _1, _2, remote_socket));
    }
    else {
        std::cerr << ec.message() << std::endl;
    }
}

void Socks5Proxy::sendDataToServer(const boost::system::error_code& ec, std::size_t length, std::shared_ptr<boost::asio::ip::tcp::socket> remote_socket) {
    if (!ec) {
        std::cout << "Data transfer to server " << remote_socket->remote_endpoint() << std::endl;
        async_write(*remote_socket, buffer(buffer_data, length), boost::bind(&Socks5Proxy::startDataTransfer, this, _1, _2, remote_socket));
    } else {
        // client_socket.close();
        // remote_socket->close();
        std::cout << "async_read_some from client socket in startDataTransfer error: " << ec.message() << std::endl; 
    }
}

void Socks5Proxy::sendDataToClient(const boost::system::error_code& ec, std::size_t length, std::shared_ptr<boost::asio::ip::tcp::socket> remote_socket) {
    if (!ec) {
        std::cout << "Data transfer to client " << client_socket.remote_endpoint() << std::endl;
        async_write(client_socket, buffer(buffer_data, length), boost::bind(&Socks5Proxy::startDataTransfer, this, _1, _2, remote_socket));
    } else {
        // client_socket.close();
        // remote_socket->close();
        std::cout << "async_read_some from remote socket in startDataTransfer error: " << ec.message() << std::endl; 
    }
}
