#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using namespace std;
using namespace boost::asio::ip;

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            cerr << "Usage: " << argv[0] << " <server_address> <server_port>" << endl;
            return 1;
        }

        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::socket socket(io_context);

        tcp::resolver::results_type endpoints = resolver.resolve(argv[1], argv[2]);

        boost::asio::connect(socket, endpoints);

        socket.close();
        // string message = "Hello, server!";
        // boost::asio::write(socket, boost::asio::buffer(message));

        // // Получаем ответ от сервера
        // char buffer[128];
        // boost::system::error_code error;
        // size_t bytes_received = socket.read_some(boost::asio::buffer(buffer), error);

        // if (!error) {
        //     cout << "Ответ от сервера: " << string(buffer, bytes_received) << endl;
        // } else {
        //     cerr << "Ошибка при получении ответа: " << error.message() << endl;
        // }

        // while (1) {
        //     boost::array<char, 128> buf;
        //     boost::system::error_code error;

        //     size_t len = socket.read_some(boost::asio::buffer(buf), error);

        //     if (error == boost::asio::error::eof)
        //         break;
        //     else if (error)
        //         throw boost::system::system_error(error);

        //     cout.write(buf.data(), len);
        // }
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }

    return 0;
}