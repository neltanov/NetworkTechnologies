#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include <iostream>

class ClientInfo {
public:
    ClientInfo();

    uint64_t getTotalBytesReceived() const;

    void addBytesReceived(std::size_t bytesRead);

private:
    uint64_t totalBytesReceived;
};

#endif // CLIENT_INFO_H