#include "../include/proxy_server.h"


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    io_context io;
    Socks5Proxy proxy(io, atoi(argv[1]));
    io.run();

    return 0;
}