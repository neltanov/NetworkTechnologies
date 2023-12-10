#include <iostream>
#include <string>
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
    if (!ec && length == 2 && connection->data()[0] == VERSION) {
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
        connection->data()[0] = VERSION;  // SOCKS5 version
        connection->data()[1] = NO_AUTH;  // No authentification method
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
    if (!ec && length == 3 && connection->data()[0] == VERSION && connection->data()[1] == CONNECT && connection->data()[2] == RSV) {
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
        if (address_type == IP_V4) { // IPv4 address
            // Read 4 bytes address and 2 bytes port
            async_read(connection->getSocket(), buffer(connection->data(), 6), bind(&Socks5Proxy::readIPv4Address, this, _1, _2, connection));
        }
        else if (address_type == DOMAIN_NAME) { // Domain name
            async_read(connection->getSocket(), buffer(connection->data(), 1), bind(&Socks5Proxy::getDomainLength, this, _1, _2, connection));
        }
        else if (address_type == IP_V6) { // IPv6 address
            async_read(connection->getSocket(), buffer(connection->data(), 18), bind(&Socks5Proxy::readIPv6Address, this, _1, _2, connection));
        }
    }
    else {
        std::cerr << "Address handling error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::readIPv4Address(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec && length == 6) {
        ip::address_v4::bytes_type ip_address_bytes;
        memcpy(ip_address_bytes.data(), &connection->data()[0], 4);
        ip::address_v4 ip(ip_address_bytes);

        uint16_t port;
        memcpy(&port, &connection->data()[4], 2);
        port = ntohs(port);

        auto remote_socket = std::make_shared<tcp::socket>(io_ctx);
        
        remote_socket->async_connect(tcp::endpoint(ip, port), bind(&Socks5Proxy::sendServerResponse, this, _1, connection, remote_socket, tcp::endpoint(ip, port)));
    }
    else {
        std::cerr << "IPv4 address reading error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::readIPv6Address(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec && length == 18) {
        ip::address_v6::bytes_type ip_address_bytes;
        memcpy(ip_address_bytes.data(), &connection->data()[0], 16);
        ip::address_v6 ip(ip_address_bytes);

        uint16_t port;
        memcpy(&port, &connection->data()[16], 2);
        port = ntohs(port);

        auto remote_socket = std::make_shared<tcp::socket>(io_ctx);
        std::cout << tcp::endpoint(ip, port) << std::endl;

        remote_socket->async_connect(tcp::endpoint(ip, port), bind(&Socks5Proxy::sendServerResponse, this, _1, connection, remote_socket, tcp::endpoint(ip, port)));
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

unsigned char Socks5Proxy::getConnectionError(const error_code& ec) {
    if (ec == error::network_unreachable) {
        return NETWORK_UNREACHABLE;
    } if (ec == error::host_unreachable) {
        return HOST_UNREACHABLE;
    } if (ec == error::connection_refused) {
        return CONNECTION_REFUSED;
    } if (ec == error::timed_out) {
        return TTL_EXPIRED;
    } /* if (cmd != CONNECT) {
        return COMMAND_NOT_SUPPORTED;
    } */ if (ec == error::address_family_not_supported) {
        return ADDRESS_TYPE_NOT_SUPPORTED;
    }
    return SUCCESS;
}

void Socks5Proxy::sendServerResponse(const error_code& ec, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket, tcp::endpoint remote_endpoint) {
    connection->data()[0] = VERSION;                 // SOCKS version
    connection->data()[1] = getConnectionError(ec);  // Status
    connection->data()[2] = RSV;                     // Reserved byte

    ip::address address = remote_endpoint.address();
    uint16_t port = htons(remote_endpoint.port());

    if (address.is_v4()) {
        connection->data()[3] = IP_V4;
        ip::address_v4::bytes_type address_bytes = address.to_v4().to_bytes();

        for (size_t i = 0; i < address_bytes.size(); i++) {
            connection->data()[i + 4] = address_bytes[i];
        }

        memcpy(&connection->data()[address_bytes.size() + 4], &port, 2);

        if (!ec) {
            std::cout << "Proxy connected to server " << remote_socket->remote_endpoint() << " successfully" << std::endl;
            async_write(connection->getSocket(), buffer(connection->data(), address_bytes.size() + 6), 
                    bind(&Socks5Proxy::startDataTransfer, this, _1, _2, connection, remote_socket));
        }
        else {
            std::cout << ec.message() << std::endl;
            error_code er_code;
            async_write(connection->getSocket(), buffer(connection->data(), address_bytes.size() + 6), bind(&ConnectionPool::addConnection, &this->connection_pool, connection));
            // connection_pool.addConnection(connection);
        }
    }
    else if (address.is_v6()) {
        connection->data()[3] = IP_V6;

        ip::address_v6::bytes_type address_bytes = address.to_v6().to_bytes();

        for (size_t i = 0; i < address_bytes.size(); i++) {
            connection->data()[i + 4] = address_bytes[i];
        }

        memcpy(&connection->data()[address_bytes.size() + 4], &port, 2);

        if (!ec) {
            std::cout << "Proxy connected to server " << remote_socket->remote_endpoint() << " successfully" << std::endl;

            async_write(connection->getSocket(), buffer(connection->data(), address_bytes.size() + 6), 
                    bind(&Socks5Proxy::startDataTransfer, this, _1, _2, connection, remote_socket));
        }
        else {
            std::cout << ec.message() << std::endl;
            error_code er_code;
            async_write(connection->getSocket(), buffer(connection->data(), address_bytes.size() + 6), bind(&ConnectionPool::addConnection, &this->connection_pool, connection));
            // connection_pool.addConnection(connection);
        }
    }

}

void Socks5Proxy::startDataTransfer(const error_code& ec, std::size_t, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            connection->getSocket().async_wait(tcp::socket::wait_read, bind(&Socks5Proxy::readDataFromClient, this, _1, connection, remote_socket));
            // remote_socket->async_wait(tcp::socket::wait_read, bind(&Socks5Proxy::readDataFromServer, this, _1, connection, remote_socket));
        }
        else {
            connection_pool.addConnection(connection);

            error_code er_code;
            if (remote_socket->is_open()) {
                remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
                remote_socket->close(er_code);
            }
        }
    }
    catch (boost::system::system_error& e) {
        std::cerr << "startDataTransfer:" << e.what() << std::endl;
    }
}

void Socks5Proxy::readDataFromClient(const error_code& ec, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            std::vector<char> data(4096);
            std::cout << "readDataFromClient" << std::endl;
            connection->getSocket().async_read_some(buffer(data), bind(&Socks5Proxy::writeDataToServer, this, _1, _2, connection, remote_socket, data));                
        } else {
            connection_pool.addConnection(connection);
            error_code er_code;
            if (remote_socket->is_open()) {
                remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
                remote_socket->close(er_code);
            }
        }
    }
    catch (boost::system::system_error& e) {
        std::cerr << "readDataFromClient: " << e.what() << std::endl;
    }
}


void Socks5Proxy::readDataFromServer(const error_code& ec, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            std::vector<char> data(4096);
            std::cout << "readDataFromServer" << std::endl;

            remote_socket->async_read_some(buffer(data), bind(&Socks5Proxy::writeDataToClient, this, _1, _2, connection, remote_socket, data));                
        } else {
            connection_pool.addConnection(connection);
            error_code er_code;
            if (remote_socket->is_open()) {
                remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
                remote_socket->close(er_code);
            }
        }
    }
    catch (boost::system::system_error& e) {
        std::cerr << "readDataFromClient: " << e.what() << std::endl;
    }
}

void Socks5Proxy::writeDataToServer(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket, std::vector<char>& data) {
    try {
        if (!ec) {
            std::cout << "writeDataToServer " << length << std::endl;
            remote_socket->async_write_some(buffer(data, length), bind(&Socks5Proxy::readDataFromServer, this, _1, connection, remote_socket));
        } else {
            connection_pool.addConnection(connection);
            error_code er_code;
            if (remote_socket->is_open()) {
                remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
                remote_socket->close(er_code);
            }
        }
    }
    catch (boost::system::system_error& e) {
        std::cerr << "readDataFromClient: " << e.what() << std::endl;
    }
}


void Socks5Proxy::writeDataToClient(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket, std::vector<char>& data) {
    try {
        if (!ec) {
            std::cout << "writeDataToClient" << length << std::endl;
            connection->getSocket().async_write_some(buffer(data, length), bind(&Socks5Proxy::startDataTransfer, this, _1, _2, connection, remote_socket));
        } else {
            connection_pool.addConnection(connection);
            error_code er_code;
            if (remote_socket->is_open()) {
                remote_socket->shutdown(tcp::socket::shutdown_both, er_code);
                remote_socket->close(er_code);
            }
        }
    }
    catch (boost::system::system_error& e) {
        std::cerr << "readDataFromClient: " << e.what() << std::endl;
    }
}
