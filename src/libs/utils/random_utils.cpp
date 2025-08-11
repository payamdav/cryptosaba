#include "random_utils.hpp"
#include <random>

namespace utils {

    double random_double() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0, 1.0);
        return dis(gen);
    }

    double random_double(double min, double max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(min, max);
        return dis(gen);
    }

    int random_int(int min, int max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }

    string random_string() {
        return to_string(random_double());
    }

    string random_string(size_t length) {
        static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        string result;
        result.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            result += alphanum[random_int(0, sizeof(alphanum) - 2)];
        }
        return result;
    }

}
