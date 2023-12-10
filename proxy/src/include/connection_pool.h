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
        // std::cout << "Connection " << connection->getSocket().remote_endpoint() << " closed" << std::endl;
        if (connection->getSocket().is_open()) {
            connection->close();
            connections.push_back(connection);
            pool_size++;
        }
    }

    std::shared_ptr<Connection> getConnection() {
        if (connections.empty()) {
            return std::make_shared<Connection>(io_ctx);
        } else {
            auto connection = connections.back();
            connections.pop_back();
            pool_size--;
            return connection;
        }
    }

    size_t size() {
        return pool_size;
    }

private:
    io_context& io_ctx;
    std::vector<std::shared_ptr<Connection>> connections;
    std::size_t pool_size = 0;
    size_t i = 0;
};

#endif // SOCKET_POOL_H