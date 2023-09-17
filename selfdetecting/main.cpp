#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::placeholders;

class UDPMulticast {
public:
    UDPMulticast(io_service& io_service,
                 const address& multicast_address,
                 unsigned short multicast_port)
        : socket_(io_service),
          multicast_endpoint_(multicast_address, multicast_port),
          timer_(io_service) {
        address listen_address = make_address("0.0.0.0");

        // Открываем сокет для приема сообщений
        socket_.open(multicast_endpoint_.protocol());
        socket_.set_option(udp::socket::reuse_address(true));
        socket_.bind(udp::endpoint(listen_address, multicast_port));

        // Добавляем сокет к multicast-группе
        socket_.set_option(multicast::join_group(multicast_address));

        // Запускаем таймер для отправки сообщений
        timer_.expires_from_now(boost::posix_time::seconds(1));
        timer_.async_wait(boost::bind(&UDPMulticast::sendUDPMessage, this));
    }

    void sendUDPMessage() {
        string message = "Hello, I am a copy of myself!";
        socket_.async_send_to(
            buffer(message),
            multicast_endpoint_,
            bind(&UDPMulticast::handleSend, this, message, _1, _2));
    }

    void handleSend(const string& message,
                    const boost::system::error_code& error,
                    size_t bytes_transferred) {
        if (!error) {
            cout << "Sent message: " << message << endl;
            timer_.expires_from_now(boost::posix_time::seconds(1));
            timer_.async_wait(boost::bind(&UDPMulticast::sendUDPMessage, this));
        } else {
            cerr << "Error sending UDP message: " << error.message() << endl;
        }
    }

private:
    udp::socket socket_;
    udp::endpoint multicast_endpoint_;
    deadline_timer timer_;
};

int main() {
    io_service io_service;
    address multicast_address = make_address("127.0.0.1"); // Замените на свой адрес
    unsigned short multicast_port = 12432;

    UDPMulticast udp_multicast(io_service, multicast_address, multicast_port);
    io_service.run();

    return 0;
}
