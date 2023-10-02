#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using namespace std;
using namespace boost::asio::ip;

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: client <host>" << endl;
            return 1;
        }

        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(argv[1], "daytime");

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        while (1) {
            boost::array<char, 128> buf;
            boost::system::error_code error;

            size_t len = socket.read_some(boost::asio::buffer(buf), error);

            if (error == boost::asio::error::eof)
                break;
            else if (error)
                throw boost::system::system_error(error);

            cout.write(buf.data(), len);
        }
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }

    return 0;
}