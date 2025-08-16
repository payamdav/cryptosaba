#include "linear_regression.hpp"

using namespace std;


LinearRegressionResult linear_regression(
    std::vector<double>::const_iterator x_begin,
    std::vector<double>::const_iterator x_end,
    std::vector<double>::const_iterator y_begin,
    std::vector<double>::const_iterator y_end,
    bool relative_to_first
) {
    size_t n = std::distance(x_begin, x_end);
    if (n != static_cast<size_t>(std::distance(y_begin, y_end)) || n == 0) {
        throw std::invalid_argument("Incompatible vector sizes");
    }

    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_xy = 0.0;
    double sum_x_squared = 0.0;

    double x_offset = 0.0;
    if (relative_to_first && n > 0) {
        x_offset = *(x_begin);
    }
    double y_offset = 0.0;
    if (relative_to_first && n > 0) {
        y_offset = *(y_begin);
    }

    for (size_t i = 0; i < n; ++i) {
        double x = *(x_begin + i) - x_offset;
        double y = *(y_begin + i) - y_offset;
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x_squared += x * x;
    }

    double numerator_m = (n * sum_xy) - (sum_x * sum_y);
    double denominator_m = (n * sum_x_squared) - (sum_x * sum_x);

    LinearRegressionResult result;
    if (denominator_m == 0.0) {
        result.slope = std::numeric_limits<double>::quiet_NaN(); // Vertical line
        result.intercept = std::numeric_limits<double>::quiet_NaN(); // Intercept is not well-defined for a vertical line
        result.count = 0;
    } else {
        result.slope = numerator_m / denominator_m;
        result.intercept = (sum_y - result.slope * sum_x) / n;
        result.count = n;
    }
    result.x_offset = x_offset;
    result.y_offset = y_offset;
    return result;
}


LinearRegressionResult linear_regression(
    boost::circular_buffer<double>::const_iterator x_begin,
    boost::circular_buffer<double>::const_iterator x_end,
    boost::circular_buffer<double>::const_iterator y_begin,
    boost::circular_buffer<double>::const_iterator y_end,
    bool relative_to_first
) {
    size_t n = std::distance(x_begin, x_end);
    if (n != static_cast<size_t>(std::distance(y_begin, y_end)) || n == 0) {
        throw std::invalid_argument("Incompatible buffer sizes");
    }

    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_xy = 0.0;
    double sum_x_squared = 0.0;
    double x_offset = 0.0;
    double y_offset = 0.0;

    if (relative_to_first && n > 0) {
        x_offset = *(x_begin);
        y_offset = *(y_begin);
    }

    for (size_t i = 0; i < n; ++i) {
        double x = *(x_begin + i) - x_offset;
        double y = *(y_begin + i) - y_offset;
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x_squared += x * x;
    }

    double numerator_m = (n * sum_xy) - (sum_x * sum_y);
    double denominator_m = (n * sum_x_squared) - (sum_x * sum_x);

    LinearRegressionResult result;
    if (denominator_m == 0.0) {
        result.slope = std::numeric_limits<double>::quiet_NaN(); // Vertical line
        result.intercept = std::numeric_limits<double>::quiet_NaN(); // Intercept is not well-defined for a vertical line
        result.count = 0;
    } else {
        result.slope = numerator_m / denominator_m;
        result.intercept = (sum_y - result.slope * sum_x) / n;
        result.count = n;
    }
    result.x_offset = x_offset;
    result.y_offset = y_offset;
    return result;
}

LinearRegressionResult linear_regression(
    boost::circular_buffer<Trade>::const_iterator begin,
    boost::circular_buffer<Trade>::const_iterator end,
    bool relative_to_first
) {
    size_t n = std::distance(begin, end);

    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_xy = 0.0;
    double sum_x_squared = 0.0;

    double x_offset = 0.0;
    double y_offset = 0.0;

    if (relative_to_first && n > 0) {
        x_offset = (begin)->t;
        y_offset = (begin)->p;
    }

    for (size_t i = 0; i < n; ++i) {
        double x = (begin + i)->t - x_offset;
        double y = (begin + i)->p - y_offset;
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x_squared += x * x;
    }

    double numerator_m = (n * sum_xy) - (sum_x * sum_y);
    double denominator_m = (n * sum_x_squared) - (sum_x * sum_x);

    LinearRegressionResult result;
    if (denominator_m == 0.0) {
        result.slope = std::numeric_limits<double>::quiet_NaN(); // Vertical line
        result.intercept = std::numeric_limits<double>::quiet_NaN(); // Intercept is not well-defined for a vertical line
        result.count = 0;
    } else {
        result.slope = numerator_m / denominator_m;
        result.intercept = (sum_y - result.slope * sum_x) / n;
        result.count = n;
    }
    result.x_offset = x_offset;
    result.y_offset = y_offset;
    return result;

}