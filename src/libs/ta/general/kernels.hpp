#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <cmath>


using namespace std;

vector<double> get_kernel(size_t n, string type="uniform") {
    vector<double> kernel(n);
    double sum = 0.0;
    double sigma = 1.0;

    if (type == "gaussian") {
        for (size_t i = 0; i < n; ++i) {
            double x = (double)i / (double)(n - 1);
            kernel[i] = exp(-0.5 * pow((x - 0.5) / sigma, 2));
            sum += kernel[i];
        }
    } else if (type == "uniform") {
        for (size_t i = 0; i < n; ++i) {
            kernel[i] = 1.0;
            sum += kernel[i];
        }
    } else if (type == "triangular") {
        for (size_t i = 0; i < n; ++i) {
            double x = (double)i / (double)(n - 1);
            kernel[i] = 1.0 - fabs(x - 0.5);
            sum += kernel[i];
        }
    } else if (type == "ramp") {
        for (size_t i = 0; i < n; ++i) {
            double x = (double)i / (double)(n - 1);
            kernel[i] = x;
            sum += kernel[i];
        }
    } else if (type == "hamming") {
        for (size_t i = 0; i < n; ++i) {
            kernel[i] = 0.54 - 0.46 * cos(2 * M_PI * i / (n - 1));
            sum += kernel[i];
        }
    } else if (type == "hanning") {
        for (size_t i = 0; i < n; ++i) {
            kernel[i] = 0.5 * (1 - cos(2 * M_PI * i / (n - 1)));
            sum += kernel[i];
        }
    } else {
        cerr << "Unknown kernel type: " << type << endl;
        return {};
    }

    for (size_t i = 0; i < n; ++i) {
        kernel[i] /= sum;
    }

    return kernel;
}
