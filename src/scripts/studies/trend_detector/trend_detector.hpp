#pragma once

#include "../../../libs/ta/candles/candles.hpp"
#include <string>
#include <vector>
#include <cstddef>


using namespace std;

class TrendSample {
    public:
    size_t ts=0;
    double price=0.0;

    TrendSample() = default;

};


class Trend {
    public:
    size_t start_ts=0;
    size_t end_ts=0;
    double start_price=0.0;
    double end_price=0.0;

    double slope=0.0;
    double r_squared=0.0;
    double error=0.0;

    vector<Candle> candles;
    vector<TrendSample> samples;

    Trend() = default;
    void push_candle(const Candle& candle);
};


class TrendDetector {
    public:
    double max_allowed_error = 0.01; // maximum allowed error for linear regression
    vector<Trend> trends;
    Trend current_trend;
    

    TrendDetector() = default;
    void feed_from_candles(string symbol, size_t start_ts=0, size_t end_ts=0);
    void push_candle(const Candle& candle);
    void push_into_new_trend(const Candle& candle);

};