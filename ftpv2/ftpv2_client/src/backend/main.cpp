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

        path file_path = argv[1];
        string filename = file_path.filename().string();
        if (filename.length() >= 4096) {
            cerr << "Filename is too long" << endl;
            return 1;
        }

        std::ifstream input_file(argv[1], ios::binary);
        if (!input_file.is_open()) {
            cerr << "File opening failed" << endl;
            return 1;
        }

        input_file.seekg(0, ios::end);
        uint64_t file_size = static_cast<uint64_t>(input_file.tellg());
        input_file.seekg(0, ios::beg);
        if (file_size >= 1024LL * 1024 * 1024 * 1024) {
            cerr << "File size is too big" << endl;
            return 1;
        }

        io_context io_context;
        tcp::socket socket(io_context);

        tcp::endpoint server_endpoint(make_address(argv[2]), atoi(argv[3]));
        socket.connect(server_endpoint);
        
        cout << "Connected to " << server_endpoint << endl;
        cout << "File name: " << filename << endl;
        cout << "File size: " << file_size << " bytes" << endl;

        write(socket, buffer(filename + '\n'));
        write(socket, buffer(to_string(file_size) + '\n'));

        boost::asio::streambuf response_buffer;
        istream response_stream(&response_buffer);
        string response;
        
        read_until(socket, response_buffer, '\n');
        getline(response_stream, response);

        if (response != "Ready") {
            cerr << "Error: the server is not ready to receive data." << endl;
            return 1;
        }
        
        char data[8192];
        while (input_file) {
            input_file.read(data, sizeof(data));
            write(socket, buffer(data, input_file.gcount()));
        }

        read_until(socket, response_buffer, '\n');
        getline(response_stream, response);

        if (response == "Success") {
            cout << "The file has been successfully transferred to the server." << endl;
        } else {
            cerr << "Error: the file was not transferred to the server." << endl;
        }

        socket.close();
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

    return 0;
}