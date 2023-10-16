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
        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

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

        ClientInfo* client_info = new ClientInfo();

        std::thread speed_check_thread(&ClientInfo::speedCheck, client_info);
        
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

        size_t file_size = stoull(file_size_str);
        cout << "File size: " << file_size << " bytes" << endl;

        string ready_status = "Ready";
        write(socket, buffer(ready_status + '\n'));

        char data[8192];
        size_t bytes_received = 0;
        size_t total_bytes_received = 0;
        size_t remaining_bytes = file_size;
        boost::system::error_code ec;
        
        while (remaining_bytes > 0) {
            size_t buffer_size = (remaining_bytes < sizeof(data)) ? remaining_bytes : sizeof(data);
            bytes_received = read(socket, boost::asio::buffer(data, buffer_size)/* , boost::bind(&FTPv2Server::handleReading, this, _1, _2) */);
            
            output_file.write(data, static_cast<streamsize>(bytes_received));
            
            total_bytes_received += bytes_received;
            client_info->addBytesReceived(bytes_received);

            remaining_bytes -= bytes_received;
        }
        output_file.close();
        io_context.run();

        total_bytes_received = client_info->getTotalBytesReceived();

        cout << "File saved to " << file_path << endl;
        cout << "Bytes received: " << total_bytes_received << endl;

        string read_status;
        if (total_bytes_received == file_size) {
            read_status = "Success";
        } else {
            read_status = "Failure";
            boost::filesystem::remove(uploads_dir.string() + '/' + filename);
        }
        cout << "File transfrerring status: " << read_status << endl;
        write(socket, buffer(read_status + '\n'));
        client_info->stopTiming();
        speed_check_thread.join();
        delete client_info;
        
        // sleep(30);
        cout << "Connection " << socket.remote_endpoint() << " is closed" << endl << endl;
        socket.close();

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

}

size_t FTPv2Server::handleReading(boost::system::error_code& ec, size_t bytes_transferred) {
    if (ec) {
        cout << bytes_transferred << endl;
        return 0;
    }
    return -1;
}
