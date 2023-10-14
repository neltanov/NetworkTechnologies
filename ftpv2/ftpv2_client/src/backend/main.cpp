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

        std::ifstream input_file(argv[1], ios::binary);
        if (!input_file.is_open()) {
            cerr << "File opening failed" << endl;
            return 1;
        }

        io_context io_context;
        tcp::socket socket(io_context);

        tcp::endpoint server_endpoint(make_address(argv[2]), atoi(argv[3]));
        socket.connect(server_endpoint);
        
        cout << "Connected to " << server_endpoint << endl;

        path file_path = argv[1];
        
        string filename = file_path.filename().string();
        write(socket, buffer(filename + '\n'));

        input_file.seekg(0, ios::end);
        uint64_t file_size = static_cast<uint64_t>(input_file.tellg());
        input_file.seekg(0, ios::beg);
        
        cout << "File size: " << file_size << " bytes" << endl;

        stringstream file_size_stream;
        file_size_stream << file_size;
        string file_size_str = file_size_stream.str();

        write(socket, buffer(file_size_str + "\n"));

        boost::asio::streambuf response_buffer;
        
        read_until(socket, response_buffer, '\n');
        istream response_stream(&response_buffer);
        string response;
        getline(response_stream, response);

        if (response == "Ready") {
            cout << "The server is ready to receive data" << endl;
        } else {
            cerr << "Error: the server is not ready to receive data." << endl;
            return -1;
        }
        
        char data[8192];
        while (input_file) {
            input_file.read(data, sizeof(data));
            write(socket, buffer(data, input_file.gcount()));
        }

        boost::asio::streambuf success_response_buffer;
        read_until(socket, success_response_buffer, '\n');
        istream success_response_stream(&success_response_buffer);
        getline(success_response_stream, response);

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