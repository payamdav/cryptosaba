#include <iostream>
#include <string>
#include "../../../libs/utils/random_utils.hpp"


using namespace std;



void all_random_tests() {
    // Test random double
    cout << "Random double: " << utils::random_double() << endl;
    cout << "Random double (1.2 to 10.8): " << utils::random_double(1.2, 10.8) << endl;

    // Test random int
    cout << "Random int: " << utils::random_int(1, 100) << endl;

    // Test random string
    cout << "Random string: utils::random_string() : " << utils::random_string() << endl;
    cout << "Random string: utils::random_string(10) : " << utils::random_string(10) << endl;
}


int main() {
    for (int i = 0; i < 5; ++i) {
        all_random_tests();
    }
    return 0;
}
