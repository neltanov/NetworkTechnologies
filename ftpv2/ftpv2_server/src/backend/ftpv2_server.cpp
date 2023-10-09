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
        boost::asio::streambuf buffer;
        string filename;
        read_until(socket, buffer, '\n', er_code);
        if (!er_code) {
            istream filenameStream(&buffer);
            getline(filenameStream, filename);
            if (!filename.empty() && filename[filename.length() - 1] == '\n') {
                filename.erase(filename.length() - 1);
            }
        } else {
            filename = "default_filename";
            cout << "sadfdasfas" << endl;
        }

        cout << "Filename: " << filename << endl;
        
        read_until(socket, buffer, '\n');
        istream size_stream(&buffer);
        string file_size_str;
        getline(size_stream, file_size_str);

        uint64_t file_size = stoull(file_size_str);

        cout << "File size: " << file_size << " bytes" << endl;

        path file_path = uploads_dir / path(filename);
        std::ofstream output_file(file_path.string(), ios::binary);
        if (!output_file.is_open()) {
            cerr << "File opening failed: " << file_path << endl;
            return;
        }

        char data[8192];
        uint64_t bytes_read = 0;
        uint64_t total_bytes_read = 0;
        uint64_t remaining_bytes = file_size;
        boost::system::error_code ec;
        while (remaining_bytes > 0) {
            size_t bufferSize = (remaining_bytes < sizeof(data)) ? remaining_bytes : sizeof(data);
            bytes_read = read(socket, boost::asio::buffer(data, bufferSize), ec);

            if (bytes_read == 0 || ec == error::eof) {
                cerr << "EOF" << endl;
                break;
            }

            if (ec) {
                cerr << "Reading error: " << ec.message() << endl;
                break;
            }

            output_file.write(data, static_cast<streamsize>(bytes_read));
            total_bytes_read += bytes_read;
            remaining_bytes -= bytes_read;
        }
        cout << "Remaining bytes: " << remaining_bytes << endl;

        cout << "File saved to " << file_path << endl;
        cout << "Total bytes " << total_bytes_read << endl;
        if (total_bytes_read == file_size) {
            cout << "The file has been successfully transferred to the server." << endl;
            boost::asio::write(socket, boost::asio::buffer("Success\n"));
        } else {
            cout << "Error: wrong transferred bytes" << endl;
            boost::asio::write(socket, boost::asio::buffer("Failure\n"));
        }

        output_file.close();
        socket.close();

        cout << "Connection " << client_endpoint << " is closed" << endl;
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

}