#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "../include/proxy_server.h"

using namespace boost::asio;
using namespace boost::placeholders;
using namespace boost::system;
using boost::asio::ip::tcp;


Socks5Proxy::Socks5Proxy(unsigned short proxy_port)
    : io_ctx(),
      client_acceptor(io_ctx, tcp::endpoint(tcp::v4(), proxy_port)),
      socket_pool(io_ctx) {}

void Socks5Proxy::start() {
    acceptConnection();
    io_ctx.run();
}

void Socks5Proxy::acceptConnection() {
    auto client_socket_ptr = socket_pool.getSocket();
    client_acceptor.async_accept(*client_socket_ptr,
        [this, client_socket_ptr](const error_code& ec) {
            if (!ec) {
                std::cout << "New connection: " << client_socket_ptr->remote_endpoint() << std::endl;
                handleGreeting(client_socket_ptr);
            }
            acceptConnection();
        });
}

void Socks5Proxy::handleGreeting(std::shared_ptr<tcp::socket> client_socket_ptr) {
    std::cout << "Handle greeting for " << client_socket_ptr->remote_endpoint() << std::endl;

    // Reading client greeting (SOCKS version and number of authentication methods supported)
    async_read(*client_socket_ptr, buffer(buffer_data, 2), bind(&Socks5Proxy::readAuthMethods, this, _1, _2, client_socket_ptr));
}

void Socks5Proxy::readAuthMethods(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr) {
    if (!ec && length == 2 && buffer_data[0] == 5) {
        size_t count_of_methods = buffer_data[1];
        // Reading authentication methods
        async_read(*client_socket_ptr, buffer(buffer_data, count_of_methods), 
                    bind(&Socks5Proxy::sendServerChoice, this, _1, _2, count_of_methods, client_socket_ptr));
    }
    else {
        std::cerr << "Auth methods receiving error: " << ec.message() << std::endl;
        socket_pool.addSocket(client_socket_ptr);
    }
}

void Socks5Proxy::sendServerChoice(const error_code& ec, std::size_t length, std::size_t count_of_methods, std::shared_ptr<tcp::socket> client_socket_ptr) {
    std::cout << "sendServerChoice " << client_socket_ptr->remote_endpoint() << std::endl;
    
    if (!ec && length == count_of_methods) {
        buffer_data[0] = 5;  // SOCKS5 version
        buffer_data[1] = 0;  // No authentification method
        // Sending server choice
        async_write(*client_socket_ptr, buffer(buffer_data, 2), bind(&Socks5Proxy::handleConnectionRequest, this, _1, _2, client_socket_ptr));
    }
    else {
        std::cerr << "Server choice sending error: " << ec.message() << std::endl;
        socket_pool.addSocket(client_socket_ptr);
    }
}

void Socks5Proxy::handleConnectionRequest(const error_code& ec, std::size_t, std::shared_ptr<tcp::socket> client_socket_ptr) {
    std::cout << "handleConnectionRequest " << client_socket_ptr->remote_endpoint() << std::endl;
    if (!ec) {
        // Reading client connection request (Firstly, SOCKS version, command code and reserved byte)
        async_read(*client_socket_ptr, buffer(buffer_data, 3), bind(&Socks5Proxy::readAddressType, this, _1, _2, client_socket_ptr));
    }
    else {
        std::cerr << "Client connection request is failed: " << ec.message() << std::endl;
        socket_pool.addSocket(client_socket_ptr);
    }
}

void Socks5Proxy::readAddressType(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr) {
    if (!ec && length == 3 && buffer_data[0] == 5 && buffer_data[1] == 1 && buffer_data[2] == 0) {
        // Reading address type
        async_read(*client_socket_ptr, buffer(buffer_data, 1), bind(&Socks5Proxy::handleAddress, this, _1, _2, client_socket_ptr));
    }
    else {
        std::cerr << "Address type reading error: " << ec.message() << std::endl;
        socket_pool.addSocket(client_socket_ptr);
    }
}

void Socks5Proxy::handleAddress(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr) {
    if (!ec && length == 1) {
        uint8_t address_type = buffer_data[0];
        if (address_type == 1) { // IPv4 address
            // Read 4 bytes address and 2 bytes port
            async_read(*client_socket_ptr, buffer(buffer_data, 6), bind(&Socks5Proxy::readIPv4Address, this, _1, _2, client_socket_ptr));
        }
        else if (address_type == 3) { // TODO: DNS resolving
            std::cout << "DNS resolving is not done yet" << std::endl;

        }
        else if (address_type == 4) { // IPv6 address (don't need)
            std::cout << "IPv6 is not supported" << std::endl;
        }
    }
    else {
        std::cerr << "Address handling error: " << ec.message() << std::endl;
        socket_pool.addSocket(client_socket_ptr);
    }
}

void Socks5Proxy::readIPv4Address(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr) {
    if (!ec && length == 6) {
        ip::address_v4::bytes_type ipAddressBytes;
        memcpy(ipAddressBytes.data(), &buffer_data[0], 4);
        ip::address_v4 ip(ipAddressBytes);

        uint16_t port;
        memcpy(&port, &buffer_data[4], 2);
        port = ntohs(port);
        std::cout << "Remote server: " << ip.to_string() << ":" << port << std::endl;

        auto remote_socket = std::make_shared<tcp::socket>(io_ctx);
        
        remote_socket->async_connect(tcp::endpoint(ip, port), bind(&Socks5Proxy::sendServerResponse, this, _1, client_socket_ptr, remote_socket));
    }
    else {
        std::cerr << "IPv4 address reading error: " << ec.message() << std::endl;
        socket_pool.addSocket(client_socket_ptr);
    }
}

void Socks5Proxy::sendServerResponse(const error_code& ec, std::shared_ptr<tcp::socket> client_socket_ptr, std::shared_ptr<tcp::socket> remote_socket) {
    if (!ec) {
        std::cout << "Proxy connected to server " << remote_socket->remote_endpoint() << " successfully" << std::endl;
        // Response packet from server
        buffer_data[0] = 5;  // SOCKS version
        buffer_data[1] = 0;  // Success
        buffer_data[2] = 0;  // Reserved byte
        buffer_data[3] = 1;  // Address type (IPv4)
        async_write(*client_socket_ptr, buffer(buffer_data, 10), 
                    bind(&Socks5Proxy::startDataTransfer, this, _1, _2, client_socket_ptr, remote_socket));
    }
    else {
        std::cerr << "Server response sending error: " << ec.message() << std::endl;
        socket_pool.addSocket(client_socket_ptr);
        error_code er_code;
        remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
        remote_socket->close(er_code);
    }
}

void Socks5Proxy::startDataTransfer(const error_code& ec, std::size_t, std::shared_ptr<tcp::socket> client_socket_ptr, std::shared_ptr<tcp::socket> remote_socket) {
    if (!ec) {
        std::cout << "Data transfer from client " << client_socket_ptr->remote_endpoint() << std::endl;
        client_socket_ptr->async_read_some(buffer(buffer_data, max_length), bind(&Socks5Proxy::sendDataToServer, this, _1, _2, client_socket_ptr, remote_socket));

        std::cout << "Data transfer from server " << remote_socket->remote_endpoint() << std::endl;
        remote_socket->async_read_some(buffer(buffer_data, max_length), bind(&Socks5Proxy::sendDataToClient, this, _1, _2, client_socket_ptr, remote_socket));
    }
    else {
        std::cerr << "Start data transfer error: " << ec.message() << std::endl;
        socket_pool.addSocket(client_socket_ptr);
        error_code er_code;
        remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
        remote_socket->close(er_code);
    }
}

void Socks5Proxy::sendDataToServer(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr, std::shared_ptr<tcp::socket> remote_socket) {
    if (!ec) {
        std::cout << "Data transfer to server " << remote_socket->remote_endpoint() << std::endl;
        async_write(*remote_socket, buffer(buffer_data, length), bind(&Socks5Proxy::startDataTransfer, this, _1, _2, client_socket_ptr, remote_socket));
    } else {
        std::cout << "async_read_some from client socket in startDataTransfer error: " << ec.message() << std::endl;
        socket_pool.addSocket(client_socket_ptr);
        error_code er_code;
        remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
        remote_socket->close(er_code);
    }
}

void Socks5Proxy::sendDataToClient(const error_code& ec, std::size_t length, std::shared_ptr<tcp::socket> client_socket_ptr, std::shared_ptr<tcp::socket> remote_socket) {
    if (!ec) {
        std::cout << "Data transfer to client " << client_socket_ptr->remote_endpoint() << std::endl;
        async_write(*client_socket_ptr, buffer(buffer_data, length), bind(&Socks5Proxy::startDataTransfer, this, _1, _2, client_socket_ptr, remote_socket));
    } else {
        std::cout << "async_read_some from remote socket in startDataTransfer error: " << ec.message() << std::endl; 
        socket_pool.addSocket(client_socket_ptr);
        error_code er_code;
        remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
        remote_socket->close(er_code);
    }
}
