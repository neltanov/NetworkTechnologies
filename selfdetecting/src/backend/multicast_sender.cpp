#include "../include/multicast_sender.h"

MulticastSender::MulticastSender(io_service& io_service,
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


void MulticastSender::sendUdpMessage() {
    string message = unique_identifier;
    socket_.send_to(buffer(message), multicast_endpoint);

    timer.expires_from_now(boost::posix_time::seconds(1));
    timer.async_wait(boost::bind(&MulticastSender::sendUdpMessage, this));
}