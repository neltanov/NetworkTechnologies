#include "../include/multicast_receiver.h"

MulticastReceiver::MulticastReceiver(io_service& io_service,
                      const ip::address& multicast_address,
                      unsigned short multicast_port,
                      const string unique_id)
    : socket_(io_service),
        multicast_endpoint(multicast_address, multicast_port),
        unique_identifier(unique_id),
        activity_check_timer(io_service) {

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

void MulticastReceiver::printLiveCopies() {
    cout << "Live copies:" << endl;
    for (const auto& address : live_copies) {
        cout << address.to_string() << endl;
    }
}

void MulticastReceiver::startReceive() {
    socket_.async_receive_from(
        buffer(recv_buffer),
        remote_endpoint,
        boost::bind(&MulticastReceiver::handleReceive, this, _1, _2));
}

void MulticastReceiver::handleReceive(const boost::system::error_code& error, size_t bytes_received) {
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
        startActivityCheckTimer();
        startReceive();
    }
}

void MulticastReceiver::startActivityCheckTimer() {
    activity_check_timer.expires_from_now(boost::posix_time::seconds(3));
    activity_check_timer.async_wait(boost::bind(&MulticastReceiver::checkActivity, this));
}

void MulticastReceiver::checkActivity() {
    removeInactiveCopies();
    startActivityCheckTimer();
}

void MulticastReceiver::removeInactiveCopies() {
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
    printLiveCopies();
}