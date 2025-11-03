#include "trend_detector.hpp"
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/core/pubsub/pubsub.hpp"
#include "../../../libs/utils/datetime_utils.hpp"
#include "../../../libs/utils/timer.hpp"
#include <iostream>

using namespace std;

// Global instances
SymbolInfo& symbolInfo = SymbolInfo::getInstance();
Config& config = Config::getInstance();
PubSub& pubsub = PubSub::getInstance();
TrendDetector detector;

void print_symbol_info_stats(const string& title) {
    cout << "\n=== " << title << " ===" << endl;

    // Calculate pips (basis points relative to last_vwap_1h)
    double pips_1s = 0.0, pips_1m = 0.0, pips_1h = 0.0;
    if (symbolInfo.last_vwap_1h > 0) {
        pips_1s = (symbolInfo.avg_candle_size_1s.mean / symbolInfo.last_vwap_1h) * 10000;
        pips_1m = (symbolInfo.avg_candle_size_1m.mean / symbolInfo.last_vwap_1h) * 10000;
        pips_1h = (symbolInfo.avg_candle_size_1h.mean / symbolInfo.last_vwap_1h) * 10000;
    }

    cout << "Averages:" << endl;
    cout << "  avg_candle_size_1s: " << symbolInfo.avg_candle_size_1s.mean << " (" << pips_1s << " pips)" << endl;
    cout << "  avg_candle_size_1m: " << symbolInfo.avg_candle_size_1m.mean << " (" << pips_1m << " pips)" << endl;
    cout << "  avg_candle_size_1h: " << symbolInfo.avg_candle_size_1h.mean << " (" << pips_1h << " pips)" << endl;
    cout << "  avg_vol_1s:         " << symbolInfo.avg_vol_1s.mean << endl;
    cout << "Latest Values:" << endl;
    cout << "  last_price:    " << symbolInfo.last_price << endl;
    cout << "  last_vwap:     " << symbolInfo.last_vwap << endl;
    cout << "  last_vwap_1m:  " << symbolInfo.last_vwap_1m << endl;
    cout << "  last_vwap_1h:  " << symbolInfo.last_vwap_1h << endl;
    cout << endl;
}

void feed_from_candles(string symbol, size_t start_ts, size_t end_ts) {
    // Instantiate candle reader
    CandleReader reader(symbol, 1);

    // Track when SymbolInfo becomes ready
    bool was_ready = symbolInfo.ready;

    // Subscribe to published candles
    pubsub.subscribe("candle", [&was_ready](const void* data) {
        const Candle* candle = static_cast<const Candle*>(data);

        // Always push to SymbolInfo
        symbolInfo.push_candle_1s(*candle);

        // Log when SymbolInfo becomes ready
        if (!was_ready && symbolInfo.ready) {
            was_ready = true;
            cout << "\n*** SymbolInfo is now READY at " << utils::get_utc_datetime_string(candle->t) << " ***" << endl;
            print_symbol_info_stats("SymbolInfo Stats at Ready");
            cout << "Starting trend detection from this point..." << endl << endl;
        }

        // Only push to TrendDetector once SymbolInfo is ready
        if (symbolInfo.ready) {
            detector.push_candle(*candle);
        }
    });

    // Publish candles from reader
    reader.publish(start_ts, end_ts);

    if (!symbolInfo.ready) {
        cout << "\nWARNING: SymbolInfo never became ready!" << endl;
        cout << "         SymbolInfo requires 7 days of data to fill all containers." << endl;
        cout << "         No trends were detected." << endl;
    } else {
        // Print final stats
        print_symbol_info_stats("SymbolInfo Final Stats");
    }
}

void run_trend_detection_test() {
    string symbol = config.get("trend_detector_symbol");
    string start_datetime = config.get("trend_detector_start_datetime");
    string end_datetime = config.get("trend_detector_end_datetime");
    double max_error = config.get_double("trend_detector_max_error");

    size_t start_ts = utils::get_timestamp(start_datetime);
    size_t end_ts = utils::get_timestamp(end_datetime);

    cout << "Symbol: " << symbol << endl;
    cout << "Start: " << utils::get_utc_datetime_string(start_ts) << endl;
    cout << "End: " << utils::get_utc_datetime_string(end_ts) << endl;
    cout << "Max error: " << max_error << endl;
    cout << endl;

    detector.max_allowed_error = max_error;

    utils::Timer timer("Candle Publishing");
    feed_from_candles(symbol, start_ts, end_ts);
    timer.checkpoint("Candle publishing complete");

    cout << "\n=== Trend Detection Results ===" << endl;
    cout << "Total trends detected: " << detector.trends.size() << endl;

    for (size_t i = 0; i < detector.trends.size(); i++) {
        const Trend& trend = detector.trends[i];
        cout << "\nTrend " << i + 1 << ":" << endl;
        cout << "  Start: " << utils::get_utc_datetime_string(trend.start_ts) << endl;
        cout << "  End: " << utils::get_utc_datetime_string(trend.end_ts) << endl;
        cout << "  Start price: " << trend.start_price << endl;
        cout << "  End price: " << trend.end_price << endl;
        cout << "  Slope: " << trend.slope << endl;
        cout << "  Intercept: " << trend.intercept << endl;
        cout << "  R^2: " << trend.r_squared << endl;
        cout << "  Error: " << trend.error << endl;
        cout << "  Candles: " << trend.candle_count << endl;
        cout << "  ZigZag points (0.10%): " << trend.zigzag_001->size() << endl;
        cout << "  ZigZag points (0.30%): " << trend.zigzag_003->size() << endl;
    }
}

int main() {
    run_trend_detection_test();
    return 0;
}
