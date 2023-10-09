#include "../include/client_info.h"

ClientInfo::ClientInfo() : totalBytesReceived(0) {}

uint64_t ClientInfo::getTotalBytesReceived() const {
    return totalBytesReceived;
}

void ClientInfo::addBytesReceived(std::size_t bytesRead) {
    totalBytesReceived += bytesRead;
}