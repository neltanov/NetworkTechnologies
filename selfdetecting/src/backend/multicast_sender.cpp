#include "../include/multicast_sender.h"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::placeholders;

MulticastSender::MulticastSender(io_context& io_context,
                    const ip::address& multicast_address,
                    unsigned short multicast_port,
                    const string unique_id)
    :   socket_(io_context),
        multicast_endpoint(multicast_address, multicast_port),
        unique_identifier(unique_id),
        timer(io_context) {
    
    socket_.open(multicast_endpoint.protocol());

    timer.expires_from_now(boost::posix_time::seconds(1));
    timer.async_wait(boost::bind(&MulticastSender::sendUdpMessage, this));
}


void MulticastSender::sendUdpMessage() {
    string message = unique_identifier;
    socket_.send_to(buffer(message), multicast_endpoint);

    timer.expires_from_now(boost::posix_time::seconds(1));
    timer.async_wait(boost::bind(&MulticastSender::sendUdpMessage, this));
}