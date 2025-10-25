#include "live_stats.hpp"
#include <cmath>



void statistics::LiveStats::push(double value) {
    sum += value;
    sum_sq += value * value;
    count++;
    mean = sum / count;
    if (value < min) min = value;
    if (value > max) max = value;
    if (count > 1) {
        variance = (sum_sq - (sum * sum) / count) / (count - 1);
        stddev = std::sqrt(variance);
    } else {
        variance = 0.0;
        stddev = 0.0;
    }
}

void statistics::LiveStats::reset() {
    sum = 0.0;
    sum_sq = 0.0;
    count = 0;
    mean = 0.0;
    variance = 0.0;
    stddev = 0.0;
    min = std::numeric_limits<double>::infinity();
    max = -std::numeric_limits<double>::infinity();
}

statistics::LiveStatsFixed::LiveStatsFixed(size_t max_size) : buffer(max_size), max_size(max_size) {}

void statistics::LiveStatsFixed::push(double value) {
    this->reset_values();
    this->buffer.push_back(value);
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {
        double val = *it;
        sum += val;
        sum_sq += val * val;
        count++;
        if (val < min) min = val;
        if (val > max) max = val;
    }
    mean = sum / count;
    if (count > 1) {
        variance = (sum_sq - (sum * sum) / count) / (count - 1);
        stddev = std::sqrt(variance);
    } else {
        variance = 0.0;
        stddev = 0.0;
    }
}

void statistics::LiveStatsFixed::reset_values() {
    sum = 0.0;
    sum_sq = 0.0;
    count = 0;
    mean = 0.0;
    variance = 0.0;
    stddev = 0.0;
    min = std::numeric_limits<double>::infinity();
    max = -std::numeric_limits<double>::infinity();
}

void statistics::LiveStatsFixed::reset() {
    buffer.clear();
    reset_values();
}

statistics::LiveAvgPeriodic::LiveAvgPeriodic(size_t period) : period(period), buffer(period) {}

void statistics::LiveAvgPeriodic::push(double value) {
    sum += value;
    if (buffer.full()) sum -= buffer.front();
    buffer.push_back(value);
    mean = sum / buffer.size();
}

void statistics::LiveAvgPeriodic::reset() {
    sum = 0.0;
    buffer.clear();
}




