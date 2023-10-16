#include "../include/client_info.h"

using namespace std;
using namespace boost::asio;

ClientInfo::ClientInfo()
    :   io(),
        timer(io),   
        total_bytes_received(0),
        second_timer(),
        is_data_ended(false) {}

uint64_t ClientInfo::getTotalBytesReceived() const {
    return total_bytes_received;
}

void ClientInfo::addBytesReceived(size_t bytes_received) {
    current_bytes_received += bytes_received;
    total_bytes_received += bytes_received;
}

void ClientInfo::speedCheck() {
    startSpeedChecking();
    io.run();
}

void ClientInfo::startSpeedChecking() {
    if (!is_data_ended) {
        timer.expires_from_now(boost::posix_time::seconds(3));
        timer.async_wait(boost::bind(&ClientInfo::printSpeedInfo, this));
    }
}

void ClientInfo::printSpeedInfo() {
    if (current_bytes_received == 0) {
        startSpeedChecking();
        return;
    }
    pair<double, string> speed;
    double instantaneous_speed = static_cast<double>(current_bytes_received) / 3.0;
    speed = convertSpeed(instantaneous_speed);
    cout << "Instantaneous speed: " << speed.first << " " << speed.second << endl;
    current_bytes_received = 0;

    boost::timer::cpu_times elapsedTimes = second_timer.elapsed();
    boost::timer::nanosecond_type elapsedNanoseconds = elapsedTimes.wall;
    double elapsed_seconds = static_cast<double>(elapsedNanoseconds) / 1e9;
    double average_speed = static_cast<double>(total_bytes_received) / elapsed_seconds;
    speed = convertSpeed(average_speed);
    cout << "Average speed: " << speed.first << " " << speed.second << endl;
    
    startSpeedChecking();
}

pair<double, string> ClientInfo::convertSpeed(double speed_in_bytes_per_second) {
    pair<double, string> speed;
    const double bytes_in_megabyte = 1024 * 1024;
    const double bytes_in_gigabyte = 1024 * 1024 * 1024;

    if (speed_in_bytes_per_second >= bytes_in_gigabyte) {
        double speed_in_gigabytes_per_second = speed_in_bytes_per_second / bytes_in_gigabyte;
        speed.first = speed_in_gigabytes_per_second;
        speed.second = "GB/s";
    } else if (speed_in_bytes_per_second >= bytes_in_megabyte) {
        double speed_in_megabytes_per_second = speed_in_bytes_per_second / bytes_in_megabyte;
        speed.first = speed_in_megabytes_per_second;
        speed.second = "MB/s";
    } else {
        speed.first = speed_in_bytes_per_second;
        speed.second = "B/s";
    }
    return speed;
}

void ClientInfo::stopTiming() {
    is_data_ended = true;
}

ClientInfo::~ClientInfo() {
    second_timer.stop();
}
