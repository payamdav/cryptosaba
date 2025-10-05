#pragma once
#include <cstddef>
#include <string>
using namespace std;


namespace utils {
    double random_double();
    double random_double(double min, double max);
    int random_int(int min, int max);
    size_t random_size_t(size_t min, size_t max);
    string random_string();
    string random_string(size_t length);

}

