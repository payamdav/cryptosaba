#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include "../../../libs/utils/timer.hpp"


using namespace Eigen;

void fill_vectors_with_sample_random_point_for_regression_test(std::vector<double>& x, std::vector<double>& y, size_t num_points) {
    x.clear();
    y.clear();
    x.reserve(num_points);
    y.reserve(num_points);

    for (size_t i = 0; i < num_points; ++i) {
        double xi = static_cast<double>(i) / num_points * 10.0; // x from 0 to 10
        double yi = 2.0 * xi + 1.0 + ((rand() % 100) / 100.0 - 0.5); // y = 2x + 1 + noise
        x.push_back(xi);
        y.push_back(yi);
    }
}

void fill_vectors_with_sample_random_point_for_regression_test_full_random(std::vector<double>& x, std::vector<double>& y, size_t num_points) {
    x.clear();
    y.clear();
    x.reserve(num_points);
    y.reserve(num_points);

    for (size_t i = 0; i < num_points; ++i) {
        // random x and random y
        double xi = static_cast<double>(rand()) / RAND_MAX * 10.0; // random x from 0 to 10
        double yi = static_cast<double>(rand()) / RAND_MAX * 20.0; // random y from 0 to 20
        x.push_back(xi);
        y.push_back(yi);
    }
}

void find_slope_intercept_regression_without_eigen(const std::vector<double>& x, const std::vector<double>& y, double& slope, double& intercept) {
    if (x.size() != y.size() || x.empty()) {
        throw std::invalid_argument("Incompatible vector sizes");
    }

    long long n = x.size(); // Use long long for n to be safe with large numbers

    // Use long double for sums to maintain precision with a large number of points
    long double sum_x = 0.0;
    long double sum_y = 0.0;
    long double sum_xy = 0.0;
    long double sum_x_squared = 0.0;

    // Accumulate sums
    for (long long i = 0; i < n; ++i) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x_squared += x[i] * x[i];
    }

  // Calculate numerator and denominator for slope
    long double numerator_m = (n * sum_xy) - (sum_x * sum_y);
    long double denominator_m = (n * sum_x_squared) - (sum_x * sum_x);

    // Check for division by zero (occurs if all x values are the same)
    if (denominator_m == 0) {
        if (numerator_m == 0) {
            // All points are collinear, and x values are constant.
            // In this specific case, the slope is undefined, but if all y values are also the same,
            // any horizontal line can be considered a fit. We'll set slope to 0 and intercept to mean_y.
            std::cerr << "Warning: All X values are identical. Slope is undefined or vertical line." << std::endl;
            slope = 0.0; // Or some other indicator of an undefined slope
            intercept = sum_y / n;
        } else {
             // x values are constant, but y values vary. This is a vertical line, slope is infinite.
             std::cerr << "Error: All X values are identical and Y values vary. Slope is infinite (vertical line)." << std::endl;
             slope = std::numeric_limits<double>::infinity(); // Use infinity to indicate vertical line
             intercept = std::numeric_limits<double>::quiet_NaN(); // Intercept is not well-defined for a vertical line
        }
        return;
    }
    slope = static_cast<double>(numerator_m / denominator_m);
    intercept = static_cast<double>((sum_y - slope * sum_x) / n);
}


void find_slope_intercept_regression_with_eigen(const std::vector<double>& x, const std::vector<double>& y, double& slope, double& intercept) {
    if (x.size() != y.size() || x.empty()) {
        throw std::invalid_argument("Incompatible vector sizes");
    }

    long long n = x.size();

    Eigen::Map<const Eigen::VectorXd> b_map(y.data(), n);
    Eigen::MatrixXd A(n, 2);
    A.col(0) = Eigen::Map<const Eigen::VectorXd>(x.data(), n);
    A.col(1).setConstant(1.0);

    Eigen::VectorXd result = A.colPivHouseholderQr().solve(b_map);
    // Eigen::VectorXd result = A.bdcSvd(ComputeThinU | ComputeThinV).solve(b_map);

    slope = result(0);
    intercept = result(1);
}



int main() {
    utils::Timer timer;
    const int num_points = 100000;
    const int num_loops = 1000;

    std::vector<double> x, y;
    double slope1, intercept1, slope2, intercept2;

    // fill_vectors_with_sample_random_point_for_regression_test(x, y, num_points);
    fill_vectors_with_sample_random_point_for_regression_test_full_random(x, y, num_points);
    timer.checkpoint("Data Generation");


    for (int i = 0; i < num_loops; ++i) {
        find_slope_intercept_regression_without_eigen(x, y, slope1, intercept1);
    }
    timer.checkpoint("Regression without Eigen");


    for (int i = 0; i < num_loops; ++i) {
        find_slope_intercept_regression_with_eigen(x, y, slope2, intercept2);
    }
    timer.checkpoint("Regression with Eigen");

    std::cout << "Without Eigen: slope = " << slope1 << ", intercept = " << intercept1 << std::endl;
    std::cout << "With Eigen: slope = " << slope2 << ", intercept = " << intercept2 << std::endl;

    return 0;
}
