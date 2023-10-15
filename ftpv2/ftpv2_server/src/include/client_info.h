#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using namespace std;
using namespace boost::asio;

class ClientInfo {
public:
    ClientInfo();

    size_t getTotalBytesReceived() const;
    void addBytesReceived(size_t bytesRead);

    void speedCheck();
    void startSpeedChecking();
    void printSpeedInfo();

private:
    io_context io;
    deadline_timer timer;
    size_t totalBytesReceived;
};

#endif // CLIENT_INFO_H