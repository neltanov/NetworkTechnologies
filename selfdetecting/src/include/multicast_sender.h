#ifndef MULTICAST_SENDER_H
#define MULTICAST_SENDER_H

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
    MulticastSender(io_context& io_context,
                    const ip::address& multicast_address,
                    unsigned short multicast_port,
                    const string unique_id);

    void sendUdpMessage();

private:
    udp::socket socket_;
    udp::endpoint multicast_endpoint;
    string unique_identifier;
    deadline_timer timer;
};

#endif // MULTICAST_SENDER_H