#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <ranges>
#include <numeric>
#include <cmath>
#include "../../libs/ta/candles/candles.hpp"
#include "../../libs/statistics/live_stats.hpp"
#include "../../libs/core/config/config.hpp"
#include "../../libs/core/pubsub/pubsub.hpp"



using namespace std;

inline void find_high_volume_nodes() {
    Config &config = Config::getInstance();
    PubSub &pubsub = PubSub::getInstance();
    string symbol = config.get("symbol");
    size_t start_ts = config.get_timestamp("datetime1");
    size_t end_ts = config.get_timestamp("datetime2");

    vector<Candle> candles_normalized_volume;
    vector<double> normalized_volumes;
    statistics::LiveAvgPeriodic vol_avg(7*24*60*60); // 7 days in seconds

    size_t published_candles_count = 0;

    pubsub.subscribe("candle", [&](void* data) {
        Candle *candle = static_cast<Candle*>(data);
        published_candles_count++;
        vol_avg.push(candle->v);
        if (vol_avg.buffer.full()) {
            candles_normalized_volume.push_back(*candle);
            double norm_vol = candle->v / vol_avg.mean;
            normalized_volumes.push_back(norm_vol);
        }
    });

    CandleReader(symbol).publish(start_ts, end_ts);

    // Further processing can be done here with candles_normalized_volume and normalized_volumes
    cout << "Published candles count: " << published_candles_count << endl;
    cout << "Normalized volumes size: " << normalized_volumes.size() << endl;

    // Head 10
    cout << "\n=== Head 10 Normalized Volumes ===" << endl;
    for (auto val : normalized_volumes | views::take(10)) {
        cout << "  " << val << endl;
    }

    // Tail 10
    cout << "\n=== Tail 10 Normalized Volumes ===" << endl;
    for (auto val : normalized_volumes | views::reverse | views::take(10)) {
        cout << "  " << val << endl;
    }

    // Statistics for normalized_volumes
    if (!normalized_volumes.empty()) {
        auto [min_val, max_val] = ranges::minmax_element(normalized_volumes);
        double sum = accumulate(normalized_volumes.begin(), normalized_volumes.end(), 0.0);
        double avg = sum / normalized_volumes.size();

        cout << "\n=== Normalized Volumes Statistics ===" << endl;
        cout << "  Average: " << avg << endl;
        cout << "  Min:     " << *min_val << endl;
        cout << "  Max:     " << *max_val << endl;
    }

    cout << endl;

    // TODO: Implement high volume nodes detection logic here
    // definition of this function is not finished yet but postponed for later

    
}

