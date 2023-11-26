#include "../include/proxy_server.h"

using namespace std;

void checkProxyPort(int port) {
    if (port < 0 || port > std::numeric_limits<unsigned short>::max()) {
        throw out_of_range("port doesn't exist");
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: " << argv[0] << " <port>" << endl;
            return 1;
        }
        checkProxyPort(atoi(argv[1]));
        
        unsigned short port = static_cast<unsigned short>(atoi(argv[1]));
        Socks5Proxy proxy(port);
        proxy.start();
    }
    catch (out_of_range& e) {
        cerr << e.what() << endl;
    }
    catch (exception& e) {
        cerr << e.what() << endl;
    }

    return 0;
}