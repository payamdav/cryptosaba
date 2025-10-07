#include "lowess.hpp"
#include <cstddef>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

LowessResult lowess(const CandlesVector& candles, long long half_neighbors_count, long long index) {
    long long candles_count = candles.size();
    int number_of_neighbors = half_neighbors_count * 2 + 1; // total neighbors count is half_neighbors_count * 2 + 1
    long long n_right = index + half_neighbors_count;
    if (n_right >= candles_count - 1) {
        n_right = candles_count - 1;
    }
    long long n_left = n_right - number_of_neighbors + 1;
    if (n_left < 0) {
        // normally this should never happen
        n_left = 0;
    }
    double d_max = n_right - index > index - n_left ? n_right - index : index - n_left;

    // case is a weighted linear regression for number of neighbors points. points x is the index of the point, y is the price of the point. weigths are calculated by the distance from the point to the index , normalized by d_max then tricubic weight function applied.
    vector<double> weights(number_of_neighbors);
    for (long long i = n_left; i <= n_right; ++i) {
        double d = (double)((double)index - i) / d_max;
        if (d < 0.0) d = -d;
        weights[i - n_left] = 1.0 - d * d * d;
        weights[i - n_left] = weights[i - n_left] * weights[i - n_left] * weights[i - n_left];
    }

    // weights are already normalized by d_max
    double sum_w = 0.0;
    double sum_wx = 0.0;
    double sum_wy = 0.0;
    double sum_wxx = 0.0;
    double sum_wxy = 0.0;
    for (long long i = n_left; i <= n_right; ++i) {
        double x = (double)i;
        double y = candles[i].vwap;
        double w = weights[i - n_left] * candles[i].v; // weight is distance weight * volume
        sum_w += w;
        sum_wx += w * x;
        sum_wy += w * y;
        sum_wxx += w * x * x;
        sum_wxy += w * x * y;
    }
    double denom = sum_w * sum_wxx - sum_wx * sum_wx;
    if (denom != 0.0) {
        double b = (sum_w * sum_wxy - sum_wx * sum_wy) / denom;
        double a = (sum_wy - b * sum_wx) / sum_w;
        double y = a + b * index;
        return LowessResult(candles[index].t, y, sum_w);
    }
    // if denom is 0, it means all x are the same, which should never happen
    cerr << "Warning: all x are the same, using average y value" << endl;
    return LowessResult(candles[index].t, 0, sum_w);
}


vector<LowessResult> lowess(const CandlesVector& candles, long long half_neighbors_count) {
    long long candles_count = candles.size();
    vector<LowessResult> results;
    results.reserve(candles_count);
    for (long long index = 0; index < candles_count; ++index) {
        results.push_back(lowess(candles, half_neighbors_count, index));
    }
    return results;
}

void save_lowess_results_to_binary_file(const string& filename, const vector<LowessResult>& results) {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: could not open file " << filename << " for writing" << endl;
        return;
    }
    for (const auto& result : results) {
        file.write((char*)&result.ts, sizeof(result.ts));
        file.write((char*)&result.y, sizeof(result.y));
        file.write((char*)&result.w, sizeof(result.w));
    }
    file.close();
}