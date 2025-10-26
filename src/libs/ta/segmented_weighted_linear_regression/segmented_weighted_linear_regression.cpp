#include "segmented_weighted_linear_regression.hpp"
#include <cmath>

SegmentedWeightedLinearRegression::SegmentedWeightedLinearRegression(
    const vector<double>& values,
    const vector<double>& weights,
    double max_error
) {
    build_segments(values, weights, max_error);
}

void SegmentedWeightedLinearRegression::build_segments(
    const vector<double>& values,
    const vector<double>& weights,
    double max_error
) {
    if (values.empty()) return;

    size_t start = 0;
    size_t n = values.size();

    while (start < n) {
        size_t end = start + 1;
        LinearSegment best_segment;

        // Extend segment while error is below threshold
        while (end <= n) {
            LinearSegment seg = fit_segment(values, weights, start, end);

            if (seg.error > max_error && end > start + 1) {
                // Error exceeded, use previous segment
                break;
            }

            best_segment = seg;

            if (end == n) break;
            end++;
        }

        segments.push_back(best_segment);
        start = best_segment.end_idx;
    }
}

LinearSegment SegmentedWeightedLinearRegression::fit_segment(
    const vector<double>& values,
    const vector<double>& weights,
    size_t start,
    size_t end
) {
    LinearSegment seg;
    seg.start_idx = start;
    seg.end_idx = end;

    if (end <= start) {
        seg.slope = 0;
        seg.intercept = 0;
        seg.error = 0;
        return seg;
    }

    // Weighted linear regression
    double sum_w = 0, sum_wx = 0, sum_wy = 0, sum_wxx = 0, sum_wxy = 0;

    for (size_t i = start; i < end; i++) {
        double x = static_cast<double>(i);
        double y = values[i];
        double w = weights[i];

        sum_w += w;
        sum_wx += w * x;
        sum_wy += w * y;
        sum_wxx += w * x * x;
        sum_wxy += w * x * y;
    }

    if (sum_w == 0) {
        seg.slope = 0;
        seg.intercept = 0;
        seg.error = 0;
        return seg;
    }

    // Calculate slope and intercept
    double denominator = sum_w * sum_wxx - sum_wx * sum_wx;

    if (abs(denominator) < 1e-10) {
        // Nearly vertical or constant, use average
        seg.slope = 0;
        seg.intercept = sum_wy / sum_w;
    } else {
        seg.slope = (sum_w * sum_wxy - sum_wx * sum_wy) / denominator;
        seg.intercept = (sum_wy - seg.slope * sum_wx) / sum_w;
    }

    // Calculate error
    seg.error = calculate_weighted_error(values, weights, start, end, seg.slope, seg.intercept);

    return seg;
}

double SegmentedWeightedLinearRegression::calculate_weighted_error(
    const vector<double>& values,
    const vector<double>& weights,
    size_t start,
    size_t end,
    double slope,
    double intercept
) {
    double sum_weighted_sq_error = 0;
    double sum_weights = 0;

    for (size_t i = start; i < end; i++) {
        double x = static_cast<double>(i);
        double predicted = slope * x + intercept;
        double residual = values[i] - predicted;
        sum_weighted_sq_error += weights[i] * residual * residual;
        sum_weights += weights[i];
    }

    if (sum_weights == 0) return 0;

    // Return RMSE
    return sqrt(sum_weighted_sq_error / sum_weights);
}
