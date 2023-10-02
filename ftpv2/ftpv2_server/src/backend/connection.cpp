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
    std::cout << "Sent message: " << message << std::endl;
}

TCPConnection::TCPConnection(io_context& io_context) : socket_(io_context) {}

void TCPConnection::handle_write(/* const boost::system::error_code& error, size_t bytes_transferred */) {
    /* std::cout << "Handling written message" << std::endl;
    if (!error) {
        std::cout << "Bytes transferred: " << bytes_transferred << std::endl;
    } */
    /* TODO: message sending speed */
}
