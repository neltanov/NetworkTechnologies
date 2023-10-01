#include "../include/ftpv2_server.h"

FTPv4Server::FTPv4Server(boost::asio::io_context& io_context,
                    unsigned short server_port)
    :   io_context(io_context),
        acceptor(io_context, tcp::endpoint(tcp::v4(), server_port)) {

    startAccept();
}

void FTPv4Server::startAccept() {
    TCPConnection::pointer new_connection = TCPConnection::create(io_context);
    acceptor.async_accept(new_connection->socket(), boost::bind(&FTPv4Server::handleAccept, this, new_connection, boost::asio::placeholders::error));
}

void FTPv4Server::handleAccept(TCPConnection::pointer new_connection, const boost::system::error_code &error) {
    if (!error) {
        new_connection->start();
    }

    startAccept();
}
