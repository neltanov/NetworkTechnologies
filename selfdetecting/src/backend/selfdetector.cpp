#include "../include/multicast_receiver.h"
#include "../include/multicast_sender.h"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::placeholders;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <multicast_address> <multicast_port> <unique_identifier>" << endl;
        return 1;
    }

    io_context io_context;
    address multicast_address = make_address(argv[1]);
    unsigned short multicast_port = atoi(argv[2]);
    string unique_identifier = argv[3];

    MulticastSender sender(io_context, multicast_address, multicast_port, unique_identifier);
    MulticastReceiver receiver(io_context, multicast_address, multicast_port, unique_identifier);

    thread sender_thread([&] { io_context.run(); });
    thread receiver_thread([&] { io_context.run(); });

    sender_thread.join();
    receiver_thread.join();

    return 0;
}