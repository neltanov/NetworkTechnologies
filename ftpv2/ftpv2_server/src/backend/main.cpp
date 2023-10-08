#include "../include/ftpv2_server.h"

using namespace std;

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: " << argv[0] << " <port>" << endl;
            return 1;
        }
        FTPv2Server server(atoi(argv[1]));
        server.start();
    }
    catch (exception& e) {
        cerr << e.what() << endl;
    }
    return 0;
}