#include "timer.hpp"
#include <chrono>
#include <iostream>


utils::Timer::Timer(string name) {
    this->name = name;
    this->reset();
}

void utils::Timer::reset() {
    start = std::chrono::high_resolution_clock::now();
    last = start;
}


void utils::Timer::checkpoint(string checkpoint_name) {

    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last);
    auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(now - start);
    if (name.size() > 0) {
        std::cout << " Timer: " << name << " ";
    }
    if (checkpoint_name.size() > 0) {
        std::cout << "Checkpoint: " << checkpoint_name << " ";
    }
    std::cout << "Duration: " << duration.count() << "s Laps  --  " << total_duration.count() << "s From Start." << std::endl;
    last = now;
}

