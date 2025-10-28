#pragma once

#include "../candles/candles.hpp"
#include <cstddef>
#include <vector>
#include <string>

using namespace std;

// calculate lowess smooth from scratch
// lowess is weighted ( each point has a weight )
// this lowess works for candles. all candles are evenly spaced with no gaps.
// volume of each candle is the weight of the point.


struct LowessResult {
    size_t ts;
    double y;
    double w; // weights

    LowessResult(size_t ts, double y, double w) : ts(ts), y(y), w(w) {}

};

LowessResult lowess(const CandlesVector& candles, long long half_neighbors_count, long long index); // calculate lowess smooth for a given index ( only one point is returned ) - neighbors count is half_neighbors_count * 2 + 1
vector<LowessResult> lowess(const CandlesVector& candles, long long half_neighbors_count);

// New overloads that work with vector<double> for y values and weights
LowessResult lowess(const vector<double>& y_values, const vector<double>& weights, const vector<size_t>& timestamps, long long half_neighbors_count, long long index);
vector<LowessResult> lowess(const vector<double>& y_values, const vector<double>& weights, const vector<size_t>& timestamps, long long half_neighbors_count);

void save_lowess_results_to_binary_file(const string& filename, const vector<LowessResult>& results);