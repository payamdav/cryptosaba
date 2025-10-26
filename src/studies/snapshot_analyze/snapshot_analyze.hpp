#pragma once
#include "../../libs/ta/candles/candles.hpp"
#include "../../libs/utils/datetime_utils.hpp"
#include "../../libs/statistics/live_stats.hpp"
#include "../../libs/ta/segmented_weighted_linear_regression/segmented_weighted_linear_regression.hpp"
#include <string>
#include <vector>
#include <cstddef>
#include <iostream>

using namespace std;

class SnapshotAnalyze {
public:
    string symbol;
    size_t ts_seconds;
    size_t ts_ms;

    CandlesVector l7d;
    CandlesVector l1d;
    CandlesVector n1d;
    CandlesVector l1dn1d;

    double current_vwap;
    double avg_candle_size_1w;
    double avg_volume_1w;
    vector<double> volume_normalized;        // l1d normalized volumes
    vector<double> prices_offsetted_scaled;  // l1d offsetted/scaled prices
    vector<double> l1dn1d_volume_normalized;       // l1dn1d normalized volumes
    vector<double> l1dn1d_prices_offsetted_scaled; // l1dn1d offsetted/scaled prices

    vector<LinearSegment> segments;            // volume-weighted

    static constexpr size_t ONE_SECOND_MS = 1000;
    static constexpr size_t ONE_MINUTE_MS = 60 * ONE_SECOND_MS;
    static constexpr size_t ONE_HOUR_MS = 60 * ONE_MINUTE_MS;
    static constexpr size_t ONE_DAY_MS = 24 * ONE_HOUR_MS;
    static constexpr size_t ONE_WEEK_MS = 7 * ONE_DAY_MS;

    SnapshotAnalyze(const string& symbol, size_t ts_seconds);
    void analyze();
    void print_summary();
    void export_to_binary();
    void export_segments();

private:
    void load_candles();
    void calculate_avg_candle_size();
    void calculate_avg_volume();
    void normalize_volumes();
    void offset_and_scale_prices();
    void set_current_vwap();
    void calculate_segments();
};
