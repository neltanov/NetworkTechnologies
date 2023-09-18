# NetworkTechnologies
# Selfdetector User Guide

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


Building a project with CMake (you are in selfdetecting path):

1) cd build
2) cmake ..
3) cmake --build .

Launch: \
./selfdetecting <multicact_address> <port> <unique_identifier>

<multicast_address> 
    - for IPv4 addresses: from 224.0.0.0 to 239.255.255.255 \
    - for IPv6 addresses: adresses which begins with FF00:: \
<port>              - from 1024 to 49151 \
<unique_identifier> - any identifier of your copy