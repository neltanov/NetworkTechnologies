#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/timer/timer.hpp>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

class ClientInfo {
public:
    ClientInfo(tcp::endpoint endpoint);

    size_t getTotalBytesReceived() const;
    void addBytesReceived(size_t bytesRead);
    void speedCheck();
    void startSpeedChecking();
    void printSpeedInfo();
    pair<double, string> convertSpeed(double speed_in_bytes_per_second);
    void stopTiming();

    ~ClientInfo();
private:
    io_context io;
    deadline_timer timer;
    size_t current_bytes_received;
    size_t total_bytes_received;
    boost::timer::auto_cpu_timer second_timer;
    bool is_data_ended;
    tcp::endpoint remote_endpoint;
};

#endif // CLIENT_INFO_H