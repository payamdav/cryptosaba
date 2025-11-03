#pragma once

#include <cstddef>
#include <limits>
#include <boost/circular_buffer.hpp>


namespace statistics {
    class LiveStats {
        public:
            size_t count = 0;
            double sum = 0.0;
            double sum_sq = 0.0;
            double mean = 0.0;
            double variance = 0.0;
            double stddev = 0.0;
            double min = std::numeric_limits<double>::infinity();
            double max = -std::numeric_limits<double>::infinity();
            void push(double value);
            void reset();
    };


    class LiveStatsFixed {
        public:
            boost::circular_buffer<double> buffer;
            size_t max_size;
            size_t count = 0;
            double sum = 0.0;
            double sum_sq = 0.0;
            double mean = 0.0;
            double variance = 0.0;
            double stddev = 0.0;
            double min = std::numeric_limits<double>::infinity();
            double max = -std::numeric_limits<double>::infinity();

            LiveStatsFixed(size_t max_size);
            void push(double value);
            void reset_values();
            void reset();
    };

    class LiveAvgPeriodic {
        public:
            static constexpr double epsilon = 1e-10;
            double sum = 0.0;
            double mean = 0.0;
            size_t period;
            boost::circular_buffer<double> buffer;
            bool non_zero_only;

            LiveAvgPeriodic(size_t period=10, bool non_zero_only=false);
            void push(double value);
            void reset();
    };

} // namespace statistics

