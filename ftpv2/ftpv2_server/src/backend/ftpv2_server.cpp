#include "../include/ftpv2_server.h"

using namespace std;
using namespace boost::asio;
using namespace boost::placeholders;
using namespace boost::filesystem;
using boost::asio::ip::tcp;

FTPv2Server::FTPv2Server(int server_port)
    :   io_context(),
        acceptor(io_context, tcp::endpoint(tcp::v4(), server_port)),
        uploads_dir("uploads") {
}

void FTPv2Server::start() {
    try {
        // std::thread speedThread(&FTPv2Server::printSpeedInfo, this);
        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            // std::lock_guard<std::mutex> lock(clients_mutex);
            // clients[&socket] = ClientInfo();

            std::thread(&FTPv2Server::handleConnection, this, std::move(socket)).detach();
        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
}

void FTPv2Server::handleConnection(tcp::socket socket) {
    try {
        cout << "New connection: " << socket.remote_endpoint() << endl;
        create_directory(uploads_dir);

        boost::asio::streambuf buffer_stream;
        string filename;

        read_until(socket, buffer_stream, '\n');

        istream filenameStream(&buffer_stream);
        getline(filenameStream, filename);

        cout << "Filename: " << filename << endl;
        
        path file_path = uploads_dir / path(filename);
        std::ofstream output_file(file_path.string(), ios::binary);
        if (!output_file.is_open()) {
            cerr << "File opening failed: " << file_path << endl;
            return;
        }

        read_until(socket, buffer_stream, '\n');

        istream string_stream(&buffer_stream);
        string file_size_str;
        getline(string_stream, file_size_str);
        cout << "In buffer remain " << buffer_stream.size() << endl;

        uint64_t file_size = stoull(file_size_str);
        cout << "File size: " << file_size << " bytes" << endl;

        string ready_status = "Ready";
        write(socket, buffer(ready_status + '\n'));

        char data[8192];
        uint64_t bytes_read = 0;
        uint64_t total_bytes_read = 0;
        uint64_t remaining_bytes = file_size;
        boost::system::error_code ec;
        while (remaining_bytes > 0) {
            size_t buffer_size = (remaining_bytes < sizeof(data)) ? remaining_bytes : sizeof(data);
            bytes_read = read(socket, boost::asio::buffer(data, buffer_size));
            
            output_file.write(data, static_cast<streamsize>(bytes_read));
            total_bytes_read += bytes_read;

            remaining_bytes -= bytes_read;
        }

        output_file.close();

        cout << "File saved to " << file_path << endl;
        cout << "Bytes received: " << total_bytes_read << endl;

        string read_status;
        if (total_bytes_read == file_size) {
            read_status = "Success";
        } else {
            read_status = "Failure";
            boost::filesystem::remove(uploads_dir.string() + '/' + filename);
        }
        cout << "File transfrerring status: " << read_status << endl;
        write(socket, buffer(read_status + '\n'));
        
        cout << "Connection " << socket.remote_endpoint() << " is closed" << endl << endl;
        socket.close();

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

}

void FTPv2Server::printSpeedInfo() {
    while (true) {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto& pair : clients) {
            const ip::tcp::socket* socket = pair.first;
            const ClientInfo& info = pair.second;

            double secondsElapsed = 3.0;
            double currentSpeed = static_cast<double>(info.getTotalBytesReceived()) / secondsElapsed;

            std::cout << "Client " << socket << ": ";
            std::cout << "Current Speed: " << currentSpeed << " bytes/second" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}