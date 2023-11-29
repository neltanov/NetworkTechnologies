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
      connection_pool(io_ctx) {}

void Socks5Proxy::start() {
    acceptConnection();
    io_ctx.run();
}

void Socks5Proxy::acceptConnection() {
    auto connection = connection_pool.getConnection();
    client_acceptor.async_accept(connection->getSocket(),
        [this, connection](const error_code& ec) {
            if (!ec) {
                std::cout << "New connection: " << connection->getSocket().remote_endpoint() << std::endl;
                handleGreeting(connection);
            }
            acceptConnection();
        });
}

void Socks5Proxy::handleGreeting(std::shared_ptr<Connection> connection) {
    // Reading client greeting (SOCKS version and number of authentication methods supported)
    async_read(connection->getSocket(), buffer(connection->data(), 2), bind(&Socks5Proxy::readAuthMethods, this, _1, _2, connection));
}

void Socks5Proxy::readAuthMethods(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec && length == 2 && connection->data()[0] == 5) {
        size_t count_of_methods = connection->data()[1];
        // Reading authentication methods
        async_read(connection->getSocket(), buffer(connection->data(), count_of_methods), 
                    bind(&Socks5Proxy::sendServerChoice, this, _1, _2, count_of_methods, connection));
    }
    else {
        std::cerr << "Auth methods receiving error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::sendServerChoice(const error_code& ec, std::size_t length, std::size_t count_of_methods, std::shared_ptr<Connection> connection) {
    if (!ec && length == count_of_methods) {
        connection->data()[0] = 5;  // SOCKS5 version
        connection->data()[1] = 0;  // No authentification method
        // Sending server choice
        async_write(connection->getSocket(), buffer(connection->data(), 2), bind(&Socks5Proxy::handleConnectionRequest, this, _1, _2, connection));
    }
    else {
        std::cerr << "Server choice sending error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::handleConnectionRequest(const error_code& ec, std::size_t, std::shared_ptr<Connection> connection) {
    if (!ec) {
        // Reading client connection request (Firstly, SOCKS version, command code and reserved byte)
        async_read(connection->getSocket(), buffer(connection->data(), 3), bind(&Socks5Proxy::readAddressType, this, _1, _2, connection));
    }
    else {
        std::cerr << "Client connection request is failed: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::readAddressType(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec && length == 3 && connection->data()[0] == 5 && connection->data()[1] == 1 && connection->data()[2] == 0) {
        // Reading address type
        async_read(connection->getSocket(), buffer(connection->data(), 1), bind(&Socks5Proxy::handleAddress, this, _1, _2, connection));
    }
    else {
        std::cerr << "Address type reading error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::handleAddress(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec && length == 1) {
        uint8_t address_type = connection->data()[0];
        if (address_type == 1) { // IPv4 address
            // Read 4 bytes address and 2 bytes port
            async_read(connection->getSocket(), buffer(connection->data(), 6), bind(&Socks5Proxy::readIPv4Address, this, _1, _2, connection));
        }
        else if (address_type == 3) { // Domain name
            async_read(connection->getSocket(), buffer(connection->data(), 1), bind(&Socks5Proxy::getDomainLength, this, _1, _2, connection));
        }
        else if (address_type == 4) { // IPv6 address (don't need)
            std::cout << "IPv6 is not supported" << std::endl;
        }
    }
    else {
        std::cerr << "Address handling error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::readIPv4Address(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec && length == 6) {
        ip::address_v4::bytes_type ipAddressBytes;
        memcpy(ipAddressBytes.data(), &connection->data()[0], 4);
        ip::address_v4 ip(ipAddressBytes);

        uint16_t port;
        memcpy(&port, &connection->data()[4], 2);
        port = ntohs(port);
        std::cout << "Remote server: " << ip.to_string() << ":" << port << std::endl;

        auto remote_socket = std::make_shared<tcp::socket>(io_ctx);
        
        remote_socket->async_connect(tcp::endpoint(ip, port), bind(&Socks5Proxy::sendServerResponse, this, _1, connection, remote_socket));
    }
    else {
        std::cerr << "IPv4 address reading error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::getDomainLength(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec && length == 1) {
        uint16_t name_length;
        memcpy(&name_length, &connection->data()[0], 1);
        std::cout << "Domain name length: " << name_length << std::endl;

        async_read(connection->getSocket(), buffer(connection->data(), name_length), bind(&Socks5Proxy::getDomainName, this, _1, _2, connection));
    }
    else {
        std::cerr << "Get domain length error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::getDomainName(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec) {
        std::cout << "Domain name length: " << length << std::endl;
        // std::cout << "Domain name: " << buffer_data << std::endl;
    }
    else {
        std::cerr << "Domain resolve error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::sendServerResponse(const error_code& ec, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    if (!ec) {
        std::cout << "Proxy connected to server " << remote_socket->remote_endpoint() << " successfully" << std::endl;
        // Response packet from server
        connection->data()[0] = 5;  // SOCKS version
        connection->data()[1] = 0;  // Success
        connection->data()[2] = 0;  // Reserved byte
        connection->data()[3] = 1;  // Address type (IPv4)
        async_write(connection->getSocket(), buffer(connection->data(), 10), 
                    bind(&Socks5Proxy::startDataTransfer, this, _1, _2, connection, remote_socket));
    }
    else {
        std::cerr << "Server response sending error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
        error_code er_code;
        remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
        remote_socket->close(er_code);
    }
}

void Socks5Proxy::startDataTransfer(const error_code& ec, std::size_t, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            std::cout << "Receiving data from client " << connection->getSocket().remote_endpoint() << std::endl;
            connection->getSocket().async_read_some(buffer(connection->data(), max_length), bind(&Socks5Proxy::sendDataToServer, this, _1, _2, connection, remote_socket));

            std::cout << "Receiving data from server " << remote_socket->remote_endpoint() << std::endl;
            remote_socket->async_read_some(buffer(connection->data(), max_length), bind(&Socks5Proxy::sendDataToClient, this, _1, _2, connection, remote_socket));
        }
        else {
            std::cerr << "Start data transfer error: " << ec.message() << std::endl;
            connection_pool.addConnection(connection);
            error_code er_code;
            remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
            remote_socket->close(er_code);
        }
    }
    catch (boost::system::system_error& e) {
        std::cerr << "startDataTransfer:" << e.what() << std::endl;
    }
}

void Socks5Proxy::sendDataToServer(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            std::cout << "Sending data to server " << remote_socket->remote_endpoint() << std::endl;
            async_write(*remote_socket, buffer(connection->data(), length), bind(&Socks5Proxy::startDataTransfer, this, _1, _2, connection, remote_socket));
        } else {
            std::cout << "async_read_some from client socket in startDataTransfer error: " << ec.message() << std::endl;
            connection_pool.addConnection(connection);
            error_code er_code;
            remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
            remote_socket->close(er_code);
        }
    }
    catch (boost::system::system_error& e) {
        std::cerr << "sendDataToServer: " << e.what() << std::endl;
    }
}

void Socks5Proxy::sendDataToClient(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            std::cout << "Sending data to client " << connection->getSocket().remote_endpoint() << std::endl;
            async_write(connection->getSocket(), buffer(connection->data(), length), bind(&Socks5Proxy::startDataTransfer, this, _1, _2, connection, remote_socket));
        } else {
            std::cout << "async_read_some from remote socket in startDataTransfer error: " << ec.message() << std::endl; 
            connection_pool.addConnection(connection);
            error_code er_code;
            remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
            remote_socket->close(er_code);
        }
    }
    catch (boost::system::system_error& e) {
        std::cerr << "sendDataToClient: " << e.what() << std::endl;
    }
}
