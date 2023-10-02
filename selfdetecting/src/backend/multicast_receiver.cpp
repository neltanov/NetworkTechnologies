#include "../include/multicast_receiver.h"

MulticastReceiver::MulticastReceiver(io_context& io_context,
                      const ip::address& multicast_address,
                      unsigned short multicast_port,
                      const string unique_id)
    : socket_(io_context),
        multicast_endpoint(multicast_address, multicast_port),
        unique_identifier(unique_id),
        activity_check_timer(io_context) {

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
                        << remote_endpoint.address().to_string() << ":" << remote_endpoint.port() << endl;

        if (sender_identifier == unique_identifier) {
            bool is_inserted = live_copies.insert(remote_endpoint).second;
            if (is_inserted) {
                cout << "New copy: " << remote_endpoint.address().to_string() << ":" << remote_endpoint.port() << endl;
                last_activity.insert(pair(remote_endpoint, boost::posix_time::second_clock::universal_time()));
                printLiveCopies();
            }
            else {
                last_activity[remote_endpoint] = boost::posix_time::second_clock::universal_time();
            }
        }

        startReceive();
    } else {
        cerr << "Error receiving UDP message: " << error.message() << endl;
        startActivityCheckTimer();
        startReceive();
    }
}

void MulticastReceiver::startActivityCheckTimer() {
    activity_check_timer.expires_from_now(boost::posix_time::seconds(4));
    activity_check_timer.async_wait(boost::bind(&MulticastReceiver::checkActivity, this));
}

void MulticastReceiver::checkActivity() {
    removeInactiveCopies();
    startActivityCheckTimer();
}

void MulticastReceiver::removeInactiveCopies() {
    auto now = boost::posix_time::second_clock::universal_time();
    set<udp::endpoint> inactive_copies;

    for (const auto& endpoint : live_copies) {
        if (now - last_activity[endpoint] > boost::posix_time::seconds(4)) {
            inactive_copies.insert(endpoint);
        }
    }

    size_t n_erased = 0;

    for (const auto& endpoint : inactive_copies) {
        n_erased = live_copies.erase(endpoint);
        last_activity.erase(endpoint);
        cout << "Removed inactive copy: " << endpoint.address().to_string() << ":" << endpoint.port() << endl;
    }

    if (n_erased != 0) {
        printLiveCopies();
    }
}

void MulticastReceiver::printLiveCopies() {
    cout << "Current copies:" << endl;
    for (const auto& endpoint : live_copies) {
        cout << endpoint.address().to_string() << ":" << endpoint.port() << endl;
    }
}