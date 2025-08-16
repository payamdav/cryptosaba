#pragma once

#include <vector>
#include <boost/circular_buffer.hpp>
#include "../../trade/tradecache.hpp"


struct LinearRegressionResult {
    double slope;
    double intercept;
    double count;
    double x_offset;
    double y_offset;
};

// Function to perform linear regression, inputs: iterators to the beginning and end of the x and y data
LinearRegressionResult linear_regression(
    std::vector<double>::const_iterator x_begin,
    std::vector<double>::const_iterator x_end,
    std::vector<double>::const_iterator y_begin,
    std::vector<double>::const_iterator y_end,
    bool relative_to_first = false
);

LinearRegressionResult linear_regression(
    boost::circular_buffer<double>::const_iterator x_begin,
    boost::circular_buffer<double>::const_iterator x_end,
    boost::circular_buffer<double>::const_iterator y_begin,
    boost::circular_buffer<double>::const_iterator y_end,
    bool relative_to_first = false
);

LinearRegressionResult linear_regression(
    boost::circular_buffer<Trade>::const_iterator begin,
    boost::circular_buffer<Trade>::const_iterator end,
    bool relative_to_first = false
);
