#include "../include/ftpv2_server.h"

using namespace std;
using namespace boost::asio;
using namespace boost::placeholders;
using boost::asio::ip::tcp;

FTPv2Server::FTPv2Server(boost::asio::io_context& io_context,
                    unsigned short server_port)
    :   io_context(io_context),
        acceptor(io_context, tcp::endpoint(tcp::v4(), server_port)) {
    startAccept();
}

void FTPv2Server::startAccept() {
    try {
        while (true) {
            ip::tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::thread(&FTPv2Server::handleConnection, this, std::move(socket)).detach();
        }
    } catch (const std::exception& e) {
        cerr << e.what() << endl;
    }
}

void FTPv2Server::handleConnection(tcp::socket socket) {
    try {
        cout << "New connection: " << socket.remote_endpoint() << endl;
        cout << "Handling connection" << endl;
        sleep(10);

        cout << "Connection " << socket.remote_endpoint() << " is closed" << endl;
        socket.close();
    } catch (const std::exception& e) {
        cerr << e.what() << endl;
    }
}