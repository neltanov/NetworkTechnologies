#include "../include/client_info.h"

using namespace std;
using namespace boost::asio;

ClientInfo::ClientInfo()
    :   io(),
        timer(io),   
        totalBytesReceived(0) {}

uint64_t ClientInfo::getTotalBytesReceived() const {
    return totalBytesReceived;
}

void ClientInfo::addBytesReceived(size_t bytesRead) {
    totalBytesReceived += bytesRead;
}

void ClientInfo::speedCheck() {
    startSpeedChecking();
    io.run();
}

void ClientInfo::startSpeedChecking() {

    timer.expires_from_now(boost::posix_time::seconds(1));
    timer.async_wait(boost::bind(&ClientInfo::printSpeedInfo, this));
}

void ClientInfo::printSpeedInfo() {
    cout << "hello from timer" << endl;
    startSpeedChecking();
}
