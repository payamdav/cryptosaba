#pragma once
#include <vector>
#include <cstddef>

using namespace std;

struct LinearSegment {
    size_t start_idx;
    size_t end_idx;
    double slope;
    double intercept;
    double error;
};

class SegmentedWeightedLinearRegression {
public:
    vector<LinearSegment> segments;

    SegmentedWeightedLinearRegression(
        const vector<double>& values,
        const vector<double>& weights,
        double max_error
    );

private:
    void build_segments(
        const vector<double>& values,
        const vector<double>& weights,
        double max_error
    );

    LinearSegment fit_segment(
        const vector<double>& values,
        const vector<double>& weights,
        size_t start,
        size_t end
    );

    double calculate_weighted_error(
        const vector<double>& values,
        const vector<double>& weights,
        size_t start,
        size_t end,
        double slope,
        double intercept
    );
};
