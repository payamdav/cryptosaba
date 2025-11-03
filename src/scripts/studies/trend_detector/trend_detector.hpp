#pragma once

#include "../../../libs/ta/candles/candles.hpp"
#include "../../../libs/ta/zigzag/zigzag.hpp"
#include "../../../libs/statistics/live_stats.hpp"
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <cstddef>


using namespace std;

class SymbolInfo {
    public:
    // Singleton instance getter
    static SymbolInfo& getInstance() {
        static SymbolInfo instance;
        return instance;
    }

    // Delete copy constructor and assignment operator
    SymbolInfo(const SymbolInfo&) = delete;
    SymbolInfo& operator=(const SymbolInfo&) = delete;

    // Public members
    Candles candles_1s;
    Candles candles_1m;
    Candles candles_1h;
    statistics::LiveAvgPeriodic avg_candle_size_1s;
    statistics::LiveAvgPeriodic avg_candle_size_1m;
    statistics::LiveAvgPeriodic avg_candle_size_1h;
    statistics::LiveAvgPeriodic avg_vol_1s;
    bool ready = false;  // True when all containers are full

    // Latest price/vwap values
    double last_price = 0.0;     // Latest 1s candle close
    double last_vwap = 0.0;      // Latest 1s candle VWAP
    double last_vwap_1m = 0.0;   // Last completed 1m candle VWAP
    double last_vwap_1h = 0.0;   // Last completed 1h candle VWAP

    // Public methods
    void push_candle_1s(const Candle& candle);

    private:
    SymbolInfo();  // Private constructor

    // Track last completed candle timestamps to detect new candles
    size_t last_candle_1m_ts = 0;
    size_t last_candle_1h_ts = 0;
};

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
    double intercept=0.0;
    double r_squared=0.0;
    double error=0.0;

    deque<Candle> candles;  // Using deque for O(1) push_back without reallocation
    vector<TrendSample> samples;
    size_t candle_count = 0; // Current candle count (can use instead of candles.size())

    unique_ptr<ZigZag> zigzag_001;  // ZigZag with 0.0010 delta
    unique_ptr<ZigZag> zigzag_003;  // ZigZag with 0.0030 delta

    Trend();
    void push_candle(const Candle& candle);
    void push_candle_old(const Candle& candle);

    private:
    // Weighted least squares running accumulators for O(1) incremental updates
    double acc_w = 0.0;      // Σwᵢ (sum of weights/volumes)
    double acc_wx = 0.0;     // Σwᵢ·xᵢ (weighted x)
    double acc_wy = 0.0;     // Σwᵢ·yᵢ (weighted y/price)
    double acc_wxx = 0.0;    // Σwᵢ·xᵢ² (weighted x-squared)
    double acc_wxy = 0.0;    // Σwᵢ·xᵢ·yᵢ (weighted cross-product)
    double acc_wyy = 0.0;    // Σwᵢ·yᵢ² (weighted y-squared)
};


class TrendDetector {
    public:
    double max_allowed_error = 0.01; // maximum allowed error for linear regression
    vector<Trend> trends;

    TrendDetector() = default;
    void push_candle(const Candle& candle);
    void push_into_new_trend(const Candle& candle);

};
