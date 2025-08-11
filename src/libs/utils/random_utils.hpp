#pragma once
#include <string>
using namespace std;


namespace utils {
    double random_double();
    double random_double(double min, double max);
    int random_int(int min, int max);
    string random_string();
    string random_string(size_t length);

}

