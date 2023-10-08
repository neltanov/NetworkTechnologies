#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include "../include/ftpv2_server.h"

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: " << argv[0] << " <port>" << endl;
            return 1;
        }

        unsigned short server_port = atoi(argv[1]);

        boost::asio::io_context io_context;
        FTPv2Server(io_context, server_port);
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}