#ifndef SOCKET_POOL_H
#define SOCKET_POOL_H

#include <boost/asio.hpp>
#include "connection.h"

using namespace boost::asio;
using boost::asio::ip::tcp;


class ConnectionPool {
public:
    ConnectionPool(io_context& io) : io_ctx(io) {}

    void addConnection(std::shared_ptr<Connection> connection) {
        connection->close();
        connections.push_back(connection);
    }

    std::shared_ptr<Connection> getConnection() {
        if (connections.empty()) {
            return std::make_shared<Connection>(io_ctx);
        } else {
            auto connection = connections.back();
            connections.pop_back();
            return connection;
        }
    }

private:
    io_context& io_ctx;
    std::vector<std::shared_ptr<Connection>> connections;

};

#endif // SOCKET_POOL_H