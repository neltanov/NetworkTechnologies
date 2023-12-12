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
                std::cout << "New client connection: " << connection->getSocket().remote_endpoint() << std::endl;
                handleGreeting(connection);
            }
            acceptConnection();
        });
}

void Socks5Proxy::handleGreeting(std::shared_ptr<Connection> connection) {
    async_read(connection->getSocket(), buffer(connection->toServerBuf(), 2), bind(&Socks5Proxy::readAuthMethods, this, _1, _2, connection));
}

void Socks5Proxy::readAuthMethods(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec && length == 2 && connection->toServerBuf()[0] == SOCKS5) {
        size_t count_of_methods = connection->toServerBuf()[1];
        async_read(connection->getSocket(), buffer(connection->toServerBuf(), count_of_methods), 
                    bind(&Socks5Proxy::sendServerChoice, this, _1, _2, count_of_methods, connection));
    }
    else {
        std::cerr << "Auth methods receiving error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::sendServerChoice(const error_code& ec, std::size_t length, std::size_t count_of_methods, std::shared_ptr<Connection> connection) {
    if (!ec && length == count_of_methods) {
        connection->toServerBuf()[0] = SOCKS5;
        connection->toServerBuf()[1] = NO_AUTH;
        async_write(connection->getSocket(), buffer(connection->toServerBuf(), 2), bind(&Socks5Proxy::handleConnectionRequest, this, _1, _2, connection));
    }
    else {
        std::cerr << "Server choice sending error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::handleConnectionRequest(const error_code& ec, std::size_t, std::shared_ptr<Connection> connection) {
    if (!ec) {
        async_read(connection->getSocket(), buffer(connection->toServerBuf(), 3), bind(&Socks5Proxy::readAddressType, this, _1, _2, connection));
    }
    else {
        std::cerr << "Client connection request is failed: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::readAddressType(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec && length == 3 && connection->toServerBuf()[0] == SOCKS5 && connection->toServerBuf()[1] == CONNECT && connection->toServerBuf()[2] == RESERVED) {
        async_read(connection->getSocket(), buffer(connection->toServerBuf(), 1), bind(&Socks5Proxy::handleAddress, this, _1, _2, connection));
    }
    else {
        std::cerr << "Address type reading error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::handleAddress(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec && length == 1) {
        uint8_t address_type = connection->toServerBuf()[0];
        if (address_type == IPv4) {
            async_read(connection->getSocket(), buffer(connection->toServerBuf(), 6), bind(&Socks5Proxy::readIPv4Address, this, _1, _2, connection));
        }
        else if (address_type == DOMAIN_NAME) {
            async_read(connection->getSocket(), buffer(connection->toServerBuf(), 1), bind(&Socks5Proxy::getDomainLength, this, _1, _2, connection));
        }
        else {
            connection_pool.addConnection(connection);
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
        memcpy(ip_address_bytes.data(), &connection->toServerBuf()[0], 4);
        ip::address_v4 ip(ip_address_bytes);

        uint16_t port;
        memcpy(&port, &connection->toServerBuf()[4], 2);
        port = ntohs(port);

        auto remote_socket = std::make_shared<tcp::socket>(io_ctx);
        
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
        memcpy(&name_length, &connection->toServerBuf()[0], 1);
        std::cout << "Domain name length: " << name_length << std::endl;

        async_read(connection->getSocket(), buffer(connection->toServerBuf(), name_length + 2), bind(&Socks5Proxy::getDomainName, this, _1, _2, connection));
    }
    else {
        std::cerr << "Get domain length error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

void Socks5Proxy::getDomainName(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection) {
    if (!ec) {
        std::cout << "Domain name and port length: " << length << std::endl;

        std::vector<char> host_name(length - 2);
        for (size_t i = 0; i < length - 2; ++i) {
            host_name[i] = static_cast<char>(connection->toServerBuf()[i]);
        }

        uint16_t dest_port;
        memcpy(&dest_port, &connection->toServerBuf()[length-2], 2);
        dest_port = ntohs(dest_port);

        if (resolved_hosts[std::string(host_name.data(), length - 2)] != ip::make_address_v4("0.0.0.0")) {
            tcp::endpoint(this->resolved_hosts[std::string(host_name.data(), length - 2)], dest_port);
        }
        tcp::resolver resolver(io_ctx);
        tcp::resolver::query query{std::string(host_name.data(), length - 2), std::to_string(dest_port)};
        // std::cout << std::string(host_name.data(), length - 2) << ' ' << dest_port << std::endl;
        // resolver.async_resolve(tcp::v4() ,query, );
        //     tcp::endpoint endpoint = *it;
        //     resolved_hosts[std::string(host_name.data(), length - 2)] = endpoint.address().to_v4();
        std::cout << "Coming soon" << std::endl;
    }
    else {
        std::cerr << "Domain resolve error: " << ec.message() << std::endl;
        connection_pool.addConnection(connection);
    }
}

unsigned char Socks5Proxy::getConnectionError(const error_code& ec) {
    if (!ec) {
        return SUCCESS;
    }
    if (ec == error::network_unreachable) {
        return NETWORK_UNREACHABLE;
    } if (ec == error::host_unreachable) {
        return HOST_UNREACHABLE;
    } if (ec == error::connection_refused) {
        return CONNECTION_REFUSED;
    } if (ec == error::timed_out) {
        return TTL_EXPIRED;
    } if (ec == error::address_family_not_supported) {
        return ADDRESS_TYPE_NOT_SUPPORTED;
    }
    return SUCCESS;
}

void Socks5Proxy::sendServerResponse(const error_code& ec, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket, tcp::endpoint remote_endpoint) {
    connection->toServerBuf()[0] = SOCKS5;
    connection->toServerBuf()[1] = getConnectionError(ec);
    connection->toServerBuf()[2] = RESERVED;
    connection->toServerBuf()[3] = IPv4;

    ip::address address = remote_endpoint.address();
    uint16_t port = htons(remote_endpoint.port());

    ip::address_v4::bytes_type address_bytes = address.to_v4().to_bytes();

    for (size_t i = 0; i < address_bytes.size(); i++) {
        connection->toServerBuf()[i + 4] = address_bytes[i];
    }

    memcpy(&connection->toServerBuf()[address_bytes.size() + 4], &port, 2);

    if (!ec) {
        std::cout << "Connected to remote server " << remote_socket->remote_endpoint() << std::endl;
        async_write(connection->getSocket(), buffer(connection->toServerBuf(), address_bytes.size() + 6), 
                bind(&Socks5Proxy::startDataTransfer, this, _1, _2, connection, remote_socket));
    }
    else {
        std::cout << "Connection to server has been failed. Reason: " << ec.message() << std::endl;
        async_write(connection->getSocket(), buffer(connection->toServerBuf(), address_bytes.size() + 6), bind(&ConnectionPool::addConnection, &this->connection_pool, connection));
    }
}

void Socks5Proxy::startDataTransfer(const error_code& ec, std::size_t, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            connection->getSocket().async_read_some(buffer(connection->toServerBuf(), max_length), bind(&Socks5Proxy::sendDataToServer, this, _1, _2, connection, remote_socket));
            remote_socket->async_read_some(buffer(connection->toClientBuf(), max_length), bind(&Socks5Proxy::sendDataToClient, this, _1, _2, connection, remote_socket));
        }
        else {
            connection_pool.addConnection(connection);
            if (remote_socket->is_open()) {
                remote_socket->shutdown(tcp::socket::shutdown_both);
                remote_socket->close();
            }
        }
    }
    catch (system_error& e) {
        std::cerr << "startDataTransfer:" << e.what() << std::endl;
    }
}

void Socks5Proxy::sendDataToServer(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            async_write(*remote_socket, buffer(connection->toServerBuf(), length), bind(&Socks5Proxy::recvDataFromClient, this, _1, _2, connection, remote_socket));
        }
        else {
            connection_pool.addConnection(connection);
            if (remote_socket->is_open()) {
                remote_socket->shutdown(tcp::socket::shutdown_both);
                remote_socket->close();
            }
        }
    }
    catch (system_error& e) {}
}

void Socks5Proxy::sendDataToClient(const error_code& ec, std::size_t length, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            async_write(connection->getSocket(), buffer(connection->toClientBuf(), length), bind(&Socks5Proxy::recvDataFromServer, this, _1, _2, connection, remote_socket));
        }
        else {
            connection_pool.addConnection(connection);
            if (remote_socket->is_open()) {
                remote_socket->shutdown(tcp::socket::shutdown_both);
                remote_socket->close();
            }
        }
    }
    catch (system_error& e) {}
}

void Socks5Proxy::recvDataFromClient(const error_code& ec, std::size_t, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            connection->getSocket().async_read_some(buffer(connection->toServerBuf(), max_length), bind(&Socks5Proxy::sendDataToServer, this, _1, _2, connection, remote_socket));
        }
        else {
            connection_pool.addConnection(connection);
            if (remote_socket->is_open()) {
                remote_socket->shutdown(tcp::socket::shutdown_both);
                remote_socket->close();
            }
        }
    }
    catch(system_error& e) {}
}

void Socks5Proxy::recvDataFromServer(const error_code& ec, std::size_t, std::shared_ptr<Connection> connection, std::shared_ptr<tcp::socket> remote_socket) {
    try {
        if (!ec) {
            remote_socket->async_read_some(buffer(connection->toClientBuf(), max_length), bind(&Socks5Proxy::sendDataToClient, this, _1, _2, connection, remote_socket));
        } 
        else {
            connection_pool.addConnection(connection);
            if (remote_socket->is_open()) {
                remote_socket->shutdown(tcp::socket::shutdown_both);
                remote_socket->close();
            }
        }
    }
    catch(system_error& e) {}
}

