#include "../include/ftpv2_server.h"

using namespace std;
using namespace boost::asio;
using namespace boost::placeholders;
using namespace boost::filesystem;
using boost::asio::ip::tcp;

FTPv2Server::FTPv2Server(int server_port)
    :   io_context(),
        acceptor(io_context, tcp::endpoint(tcp::v4(), server_port)) {
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
        tcp::endpoint client_endpoint = socket.remote_endpoint();
        cout << "New connection: " << client_endpoint << endl;

        path uploads_dir("uploads");
        if (!exists(uploads_dir))
            create_directory(uploads_dir);

        boost::system::error_code er_code;
        boost::asio::streambuf filename_buffer;
        boost::asio::read_until(socket, filename_buffer, '\n', er_code);
        string filename;
        if (!er_code) {
            istream filenameStream(&filename_buffer);
            getline(filenameStream, filename);
            if (!filename.empty() && filename[filename.length() - 1] == '\n') {
                filename.erase(filename.length() - 1);
            }
        } else {
            filename = "default_filename";
        }

        cout << "Filename: " << filename << endl;

        uint64_t file_size = 0;
        boost::asio::read(socket, boost::asio::buffer(&file_size, sizeof(file_size)));
        file_size = boost::asio::detail::socket_ops::network_to_host_long(file_size);
        cout << "File size: " << file_size << endl;

        path file_path = uploads_dir / path(filename);
        std::ofstream output_file(file_path.string(), ios::binary);
        if (!output_file.is_open()) {
            cerr << "File opening failed: " << file_path << endl;
            return;
        }

        const uint64_t buffer_size = 8192;
        char buffer[buffer_size];
        size_t bytes_read = 0;
        uint64_t bytes_remaining = file_size;
        while (bytes_remaining > 0) {
            bytes_read = socket.read_some(boost::asio::buffer(buffer, min(buffer_size, bytes_remaining)));
            if (bytes_read == 0)
                break;
            output_file.write(buffer, bytes_read);
            bytes_remaining -= bytes_read;
        }

        output_file.close();
        socket.close();

        cout << "File saved to " << file_path << endl;
        cout << "Connection " << client_endpoint << " is closed" << endl;
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

}