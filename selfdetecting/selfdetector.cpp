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
                    const string unique_id)
        : socket_(io_service),
          multicast_endpoint(multicast_address, multicast_port),
          timer(io_service),
          unique_identifier(unique_id) {
        
        socket_.open(multicast_endpoint.protocol());

        timer.expires_from_now(boost::posix_time::seconds(1));
        timer.async_wait(boost::bind(&MulticastSender::sendUdpMessage, this));
    }

    void sendUdpMessage() {
        string message = unique_identifier;
        socket_.send_to(buffer(message), multicast_endpoint);

        timer.expires_from_now(boost::posix_time::seconds(1));
        timer.async_wait(boost::bind(&MulticastSender::sendUdpMessage, this));
    }

private:
    udp::socket socket_;
    udp::endpoint multicast_endpoint;
    string unique_identifier;
    deadline_timer timer;
};

class MulticastReceiver {
public:
    MulticastReceiver(io_service& io_service,
                      const ip::address& multicast_address,
                      unsigned short multicast_port,
                      const string unique_id)
        : socket_(io_service),
          multicast_endpoint(multicast_address, multicast_port),
          activity_check_timer(io_service),
          unique_identifier(unique_id) {

        socket_.open(multicast_endpoint.protocol());
        socket_.set_option(udp::socket::reuse_address(true));
        
        if (multicast_address.is_v4()) {
            socket_.bind(udp::endpoint(ip::address_v4::any(), multicast_port));
        } else if (multicast_address.is_v6()) {
            socket_.bind(udp::endpoint(ip::address_v6::any(), multicast_port));
        }

        socket_.set_option(multicast::join_group(multicast_address));

        startReceive();

        startActivityCheckTimer();
    }

    void printLiveCopies() {
        cout << "Live copies:" << endl;
        for (const auto& address : live_copies) {
            cout << address.to_string() << endl;
        }
    }

private:
    void startReceive() {
        socket_.async_receive_from(
            buffer(recv_buffer),
            remote_endpoint,
            boost::bind(&MulticastReceiver::handleReceive, this, _1, _2));
    }

    void handleReceive(const boost::system::error_code& error, size_t bytes_received) {
        if (!error) {
            string received_message(recv_buffer.data(), bytes_received);
            string sender_identifier = received_message;

            cout << "Received message: " << received_message << " from "
                          << remote_endpoint.address().to_string() << endl;

            if (sender_identifier == unique_identifier) {
                live_copies.insert(remote_endpoint.address());
                printLiveCopies();
            }

            startReceive();
        } else {
            cerr << "Error receiving UDP message: " << error.message() << endl;

            // printLiveCopies();
            startActivityCheckTimer();
            startReceive();
        }
    }

    void startActivityCheckTimer() {
        activity_check_timer.expires_from_now(boost::posix_time::seconds(3));
        activity_check_timer.async_wait(boost::bind(&MulticastReceiver::checkActivity, this));
    }

    void checkActivity() {
        removeInactiveCopies();
        startActivityCheckTimer();
    }

    void removeInactiveCopies() {
        auto now = boost::posix_time::second_clock::universal_time();
        set<address> inactive_copies;

        for (const auto& address : live_copies) {
            if (now - last_activity[address] > boost::posix_time::seconds(3)) {
                inactive_copies.insert(address);
            }
        }

        for (const auto& address : inactive_copies) {
            live_copies.erase(address);
            last_activity.erase(address);
            cout << "Removed inactive copy: " << address.to_string() << endl;
        }
        // printLiveCopies();
    }

private:
    udp::socket socket_;
    udp::endpoint multicast_endpoint;
    udp::endpoint remote_endpoint;
    array<char, 1024> recv_buffer;
    string unique_identifier;
    set<address> live_copies;
    map<address, boost::posix_time::ptime> last_activity;
    deadline_timer activity_check_timer;
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