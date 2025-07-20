#pragma once
#include <chrono>
#include <iostream>
#include <string>

using namespace std;
    
namespace utils {

class Timer {
    public:
        string name;
        std::chrono::_V2::system_clock::time_point start, last;

        Timer(string name="");
        void reset();
        void checkpoint(string checkpoint_name="");
};

}
