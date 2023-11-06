#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "../include/proxy_server.h"

using namespace boost::asio;

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
                handleHandshake();
            } else if (ec != boost::asio::error::operation_aborted) {
                // std::cerr << "Error accepting connection: " << ec.message() << std::endl;
                // sleep(1);
            }
            acceptConnection();
        });
}

void Socks5Proxy::handleHandshake() {
    // Reading client greeting (SOCKS version and number of authentication methods supported)
    async_read(client_socket, buffer(buffer_data, 2),
        [this](const boost::system::error_code& ec, std::size_t length) {
            if (!ec && length == 2 && buffer_data[0] == 5) {
                size_t count_of_methods = buffer_data[1];
                // Reading authentication methods
                async_read(client_socket, buffer(buffer_data, count_of_methods),
                    [this, count_of_methods](const boost::system::error_code& ec, std::size_t length) {
                        if (!ec && length == count_of_methods) {
                            buffer_data[0] = 5;  // SOCKS5 version
                            buffer_data[1] = 0;  // No authentification method
                            // Sending server choice
                            async_write(client_socket, buffer(buffer_data, 2),
                                [this](const boost::system::error_code& ec, std::size_t) {
                                    if (!ec) {
                                        handleRequest();
                                    }
                                });
                        }
                    });
            }
        });
}

void Socks5Proxy::handleRequest() {
    // Reading client connection request (Firstly, SOCKS version, command code and reserved byte)
    async_read(client_socket, buffer(buffer_data, 3),
        [this](const boost::system::error_code& ec, std::size_t length) {
            if (!ec && length == 3 && buffer_data[0] == 5 && buffer_data[1] == 1 && buffer_data[2] == 0) {
                // Reading address type
                async_read(client_socket, buffer(buffer_data, 1),
                    [this](const boost::system::error_code& ec, std::size_t length) {
                        if (!ec && length == 1) {
                            uint8_t address_type = buffer_data[0];
                            // IPv4 case
                            if (address_type == 1) {
                                async_read(client_socket, buffer(buffer_data, 6),
                                    [this](const boost::system::error_code& ec, std::size_t length) {
                                        if (!ec && length == 6) {
                                            boost::asio::ip::address_v4::bytes_type ipAddressBytes;
                                            memcpy(ipAddressBytes.data(), &buffer_data[0], 4);
                                            boost::asio::ip::address_v4 ip(ipAddressBytes);

                                            uint16_t port;
                                            memcpy(&port, &buffer_data[4], 2);
                                            port = ntohs(port);
                                            std::cout << "Remote server: " << ip.to_string() << ":" << port << std::endl;

                                            auto remote_socket = std::make_shared<ip::tcp::socket>(client_socket.get_executor());
                                            
                                            remote_socket->async_connect(ip::tcp::endpoint(ip, port),
                                                [this, remote_socket](const boost::system::error_code& ec) {
                                                    if (!ec) {
                                                        // Response packet from server
                                                        buffer_data[0] = 5;  // SOCKS version
                                                        buffer_data[1] = 0;  // Success
                                                        buffer_data[2] = 0;  // Reserved byte
                                                        buffer_data[3] = 1;  // Address type (IPv4)
                                                        async_write(client_socket, buffer(buffer_data, 10),
                                                            [this, remote_socket](const boost::system::error_code& ec, std::size_t) {
                                                                if (!ec) {
                                                                    startDataTransfer(client_socket, remote_socket);
                                                                }
                                                            });
                                                    }
                                                    else {
                                                        std::cerr << ec.message() << std::endl;
                                                    }
                                                });
                                    }
                                });
                            }
                            else if (address_type == 3) { // TODO: DNS resolving
                                std::cout << "DNS resolving is not done yet" << std::endl;

                            }
                            else if (address_type == 4) { // IPv6 address (don't need)
                                std::cout << "IPv6 is not supported" << std::endl;
                            }
                        }
                    });
            }
        });
}

void Socks5Proxy::startDataTransfer(ip::tcp::socket& client_socket, std::shared_ptr<boost::asio::ip::tcp::socket> remote_socket) {
    client_socket.async_read_some(buffer(buffer_data, max_length),
        [this, &client_socket, remote_socket](const boost::system::error_code& ec, std::size_t length) {
            if (!ec) {
                async_write(*remote_socket, buffer(buffer_data, length),
                    [this, &client_socket, remote_socket](const boost::system::error_code& ec, std::size_t) {
                        if (!ec) {
                            std::cout << "Starting data transfer to " << remote_socket.get()->remote_endpoint() << std::endl;
                            startDataTransfer(client_socket, remote_socket);
                        } else {
                            // client_socket.close();
                            // remote_socket->close();
                            std::cout << "async_write to client socket in startDataTransfer error: " << ec.message() << std::endl; 
                        }
                    });
            } else {
                // client_socket.close();
                // remote_socket->close();
                std::cout << "async_read_some from client socket in startDataTransfer error: " << ec.message() << std::endl; 
            }
        });

    remote_socket->async_read_some(buffer(buffer_data, max_length),
        [this, &client_socket, remote_socket](const boost::system::error_code& ec, std::size_t length) {
            if (!ec) {
                async_write(client_socket, buffer(buffer_data, length),
                    [this, &client_socket, remote_socket](const boost::system::error_code& ec, std::size_t) {
                        if (!ec) {
                            std::cout << "Starting data transfer to " << client_socket.remote_endpoint() << std::endl;
                            startDataTransfer(client_socket, remote_socket);    
                        } else {
                            // client_socket.close();
                            // remote_socket->close();
                            std::cout << "async_write to remote socket in startDataTransfer error: " << ec.message() << std::endl; 
                        }
                    });
            } else {
                // client_socket.close();
                // remote_socket->close();
                std::cout << "async_read_some from remote socket in startDataTransfer error: " << ec.message() << std::endl; 
            }
        });
}