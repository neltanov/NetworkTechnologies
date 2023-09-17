#include <iostream>
#include <boost/array.hpp>

using namespace std;

int main() {
    boost::array<int, 4> arr = {{1, 2, 3, 4}};
    for (int i = 0; i < arr.size(); i++) {

    }
    cout << "hi" << arr[0] << endl;

    return 0;
}