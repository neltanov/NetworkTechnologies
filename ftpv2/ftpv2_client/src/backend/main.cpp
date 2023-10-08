#include <iostream>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

using namespace std;

using namespace boost::asio;
using namespace boost::system;
using namespace boost::asio::ip;
using namespace boost::filesystem;

int main(int argc, char* argv[]) {
    try {
        if (argc != 4) {
            cerr << "Usage: " << argv[0] << " <path_to_file> <server_address> <server_port>" << endl;
            return 1;
        }

        io_context io_context;
        tcp::socket socket(io_context);

        tcp::endpoint server_endpoint(make_address(argv[2]), atoi(argv[3]));
        socket.connect(server_endpoint);
        
        path file_path = argv[1];

        std::string filename = file_path.filename().string();
        boost::asio::write(socket, boost::asio::buffer(filename + '\n'));

        std::ifstream input_file(argv[1], ios::binary);
        if (!input_file.is_open()) {
            cerr << "File opening failed" << endl;
            return 1;
        }

        input_file.seekg(0, ios::end);
        streampos file_size = input_file.tellg();
        input_file.seekg(0, ios::beg);
        
        cout << "File size: " << file_size << endl; // 13

        uint64_t file_size_network_order = boost::asio::detail::socket_ops::host_to_network_long(file_size);
        boost::asio::write(socket, boost::asio::buffer(&file_size_network_order, sizeof(file_size_network_order)));

        const int buffer_size = 8192;
        char buffer[buffer_size];
        while (!input_file.eof()) {
            // streamsize read_size = min(static_cast<streamsize>(buffer_size), static_cast<streamsize>(file_size));
            input_file.read(buffer, buffer_size);
            std::streamsize bytes_read = input_file.gcount();
            socket.write_some(boost::asio::buffer(buffer, bytes_read));
        }

        socket.close();

        cout << "The file has been successfully sent" << endl;
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

    return 0;
}