#include "../include/connection.h"

TCPConnection::pointer TCPConnection::create(boost::asio::io_context& io_context) {
        return pointer(new TCPConnection(io_context));
}

tcp::socket& TCPConnection::socket() {
    return socket_;
}

void TCPConnection::start() {
    message = "hello";
    auto write_handler_callback = boost::bind(
        &TCPConnection::handle_write, shared_from_this()/* , 
        boost::asio::placeholders::error, 
        boost::asio::placeholders::bytes_transferred */
        );
    boost::asio::async_write(socket_, boost::asio::buffer(message), write_handler_callback);
}

TCPConnection::TCPConnection(io_service& io_service) : socket_(io_service) {}

void TCPConnection::handle_write(/* const boost::system::error_code& e, size_t bytes_transferred */) {
    // some actions with connection
}
