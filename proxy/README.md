# SOCKS5 Proxy server User Guide

Firstly you need to install Boost. \
For Linux Ubuntu:
1) sudo apt-get install libboost-all-dev

For Windows:
1) Download boost archive from https://www.boost.org/
2) Unpack it to a local disk C
3) Open the unpacked folder and run the bootstrap.bat file
4) After bootstrap.bat will work, a file will appear in the folder b2.exe
5) Run in command line: b2.exe toolset=gcc --prefix=<your_prefered_path> and wait from 5 minutes to 1.5 hours (depends on the "machine"). --prefix - a path which will contain build with all libraries from boost.
6) Add include/boost-1_83 and lib/ paths from build directory (prefix) to PATH.


Building a project with CMake (you are in proxy_server path):

1) mkdir build && cd build
2) cmake ..
3) cmake --build

Run:
./proxy_server <port>