#ifndef MULTICAST_RECEIVER_H
#define MULTICAST_RECEIVER_H

#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::placeholders;

class MulticastReceiver {
public:
    MulticastReceiver(io_context& io_context,
                      const ip::address& multicast_address,
                      unsigned short multicast_port,
                      const string unique_id);
    void printLiveCopies();

private:
    void startReceive();
    void handleReceive(const boost::system::error_code& error, size_t bytes_received);
    void startActivityCheckTimer();
    void checkActivity();
    void removeInactiveCopies();

private:
    udp::socket socket_;
    udp::endpoint multicast_endpoint;
    udp::endpoint remote_endpoint;
    array<char, 1024> recv_buffer;
    string unique_identifier;
    set<udp::endpoint> live_copies;
    map<udp::endpoint, boost::posix_time::ptime> last_activity;
    deadline_timer activity_check_timer;
};

#endif // MULTICAST_RECEIVER_H