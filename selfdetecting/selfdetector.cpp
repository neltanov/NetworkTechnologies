#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::placeholders;

class MulticastSender {
public:
    MulticastSender(io_service& io_service,
                    const ip::address& multicast_address,
                    unsigned short multicast_port,
                    const string& unique_identifier)
        : socket_(io_service),
          multicast_endpoint_(multicast_address, multicast_port),
          timer_(io_service),
          unique_identifier_(unique_identifier) {
        // Открываем соксет для отправки multicast сообщений
        socket_.open(multicast_endpoint_.protocol());

        // Запускаем таймер для отправки сообщений каждую секунду
        timer_.expires_from_now(boost::posix_time::seconds(1));
        timer_.async_wait(boost::bind(&MulticastSender::sendUdpMessage, this));
    }

    void sendUdpMessage() {
        string message = "Hello from " + unique_identifier_;
        socket_.send_to(buffer(message), multicast_endpoint_);

        // Запускаем таймер снова для отправки следующего сообщения
        timer_.expires_from_now(boost::posix_time::seconds(1));
        timer_.async_wait(boost::bind(&MulticastSender::sendUdpMessage, this));
    }

private:
    udp::socket socket_;
    udp::endpoint multicast_endpoint_;
    string unique_identifier_;
    deadline_timer timer_;
};

class MulticastReceiver {
public:
    MulticastReceiver(io_service& io_service,
                      const ip::address& multicast_address,
                      unsigned short multicast_port,
                      const string& unique_identifier)
        : socket_(io_service),
          multicast_endpoint_(multicast_address, multicast_port),
          activity_check_timer(io_service),
          unique_identifier_(unique_identifier) {
        // Открываем соксет для прослушивания multicast сообщений
        socket_.open(multicast_endpoint_.protocol());
        
        if (multicast_address.is_v4()) {
            socket_.set_option(udp::socket::reuse_address(true));
            socket_.bind(udp::endpoint(ip::address_v4::any(), multicast_port));
        } else if (multicast_address.is_v6()) {
            socket_.set_option(udp::socket::reuse_address(true));
            socket_.bind(udp::endpoint(ip::address_v6::any(), multicast_port));
        }

        // Добавляем соксет к multicast-группе
        socket_.set_option(multicast::join_group(multicast_address));

        // Запускаем асинхронный прием сообщений
        startReceive();

        startActivityCheckTimer();
    }

    void printLiveCopies() {
        cout << "Live copies:" << endl;
        for (const auto& address : live_copies_) {
            cout << address.to_string() << endl;
        }
    }

private:
    void startReceive() {
        socket_.async_receive_from(
            buffer(recv_buffer_),
            remote_endpoint_,
            boost::bind(&MulticastReceiver::handleReceive, this, _1, _2));
    }

    void handleReceive(const boost::system::error_code& error, size_t bytes_received) {
        if (!error) {
            string received_message(recv_buffer_.data(), bytes_received);
            string sender_identifier = received_message.substr(11); // Убираем "Hello from "

            if (sender_identifier != unique_identifier_) {
                // cout << "Received message: " << received_message << " from "
                //           << remote_endpoint_.address().to_string() << endl;
                
                // Добавляем адрес копии в список живых копий
                lock_guard<mutex> lock(mutex_);
                live_copies_.insert(remote_endpoint_.address());

                printLiveCopies();
            }

            startReceive();
        } else {
            cerr << "Error receiving UDP message: " << error.message() << endl;

            printLiveCopies();
            startActivityCheckTimer();
            startReceive();
        }
    }

    void startActivityCheckTimer() {
        activity_check_timer.expires_from_now(boost::posix_time::seconds(10)); // Проверяем каждые 10 секунд
        activity_check_timer.async_wait(boost::bind(&MulticastReceiver::checkActivity, this));
    }

    void checkActivity() {
        removeInactiveCopies();
        startActivityCheckTimer();
    }

    void removeInactiveCopies() {
        auto now = boost::posix_time::second_clock::universal_time();
        set<address> inactive_copies;

        for (const auto& address : live_copies_) {
            if (now - last_activity_[address] > boost::posix_time::seconds(10)) {
                // Если копия не была активной в течение 10 секунд, считаем ее неактивной
                inactive_copies.insert(address);
            }
        }

        for (const auto& address : inactive_copies) {
            live_copies_.erase(address);
            last_activity_.erase(address);
            cout << "Removed inactive copy: " << address.to_string() << endl;
        }
        printLiveCopies();
    }

private:
    udp::socket socket_;
    udp::endpoint multicast_endpoint_;
    udp::endpoint remote_endpoint_;
    array<char, 1024> recv_buffer_;
    string unique_identifier_;
    set<address> live_copies_;
    map<address, boost::posix_time::ptime> last_activity_;
    deadline_timer activity_check_timer;
    mutex mutex_;
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <multicast_address> <multicast_port> <unique_identifier>" << endl;
        return 1;
    }

    io_service io_service;
    address multicast_address = make_address(argv[1]);
    unsigned short multicast_port = atoi(argv[2]);
    string unique_identifier = argv[3];

    MulticastSender sender(io_service, multicast_address, multicast_port, unique_identifier);
    MulticastReceiver receiver(io_service, multicast_address, multicast_port, unique_identifier);

    thread sender_thread([&] { io_service.run(); });
    thread receiver_thread([&] { io_service.run(); });

    sender_thread.join();
    receiver_thread.join();

    return 0;
}