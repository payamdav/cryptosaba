#include "trend_detector.hpp"
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/core/pubsub/pubsub.hpp"
#include "../../../libs/utils/datetime_utils.hpp"
#include "../../../libs/utils/timer.hpp"
#include "../../../libs/utils/file_utils.hpp"
#include <iostream>
#include <fstream>

using namespace std;

// Global instances
SymbolInfo& symbolInfo = SymbolInfo::getInstance();
Config& config = Config::getInstance();
PubSub& pubsub = PubSub::getInstance();
TrendDetector detector;  // Constructor initializes Trend::symbol_info

// Containers for candles from ready point onward
vector<Candle> candles_1s_from_ready;
vector<Candle> candles_1m_from_ready;
vector<Candle> candles_1h_from_ready;

void save_candles_to_csv(const vector<Candle>& candles, const string& filename, const string& timeframe_name) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << " for writing" << endl;
        return;
    }

    // Write CSV header
    file << "timestamp,datetime,timeframe,open,high,low,close,vwap,volume,volume_sell,volume_buy,count" << endl;

    // Write candle data
    for (size_t i = 0; i < candles.size(); i++) {
        const Candle& c = candles[i];
        file << c.t << ","
             << utils::get_utc_datetime_string(c.t) << ","
             << timeframe_name << ","
             << c.o << ","
             << c.h << ","
             << c.l << ","
             << c.c << ","
             << c.vwap << ","
             << c.v << ","
             << c.vs << ","
             << c.vb << ","
             << c.n << endl;
    }

    file.close();
    cout << "Saved " << candles.size() << " candles to " << filename << endl;
}

void save_trends_to_csv(const vector<Trend>& trends, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << " for writing" << endl;
        return;
    }

    // Write CSV header
    file << "trend_id,start_ts,start_datetime,end_ts,end_datetime,start_price,end_price,"
         << "slope,intercept,r_squared,error,candle_count,zigzag_1_points,zigzag_2_points" << endl;

    // Write trend data
    for (size_t i = 0; i < trends.size(); i++) {
        const Trend& t = trends[i];
        file << i + 1 << ","
             << t.start_ts << ","
             << utils::get_utc_datetime_string(t.start_ts) << ","
             << t.end_ts << ","
             << utils::get_utc_datetime_string(t.end_ts) << ","
             << t.start_price << ","
             << t.end_price << ","
             << t.slope << ","
             << t.intercept << ","
             << t.r_squared << ","
             << t.error << ","
             << t.candle_count << ","
             << t.zigzag_1->size() << ","
             << t.zigzag_2->size() << endl;
    }

    file.close();
    cout << "Saved " << trends.size() << " trends to " << filename << endl;
}

void save_zigzag_points_to_csv(const vector<Trend>& trends, const string& filename_1, const string& filename_2) {
    // Save zigzag_1 points
    ofstream file_1(filename_1);
    if (file_1.is_open()) {
        file_1 << "trend_id,point_id,timestamp,datetime,price,is_high" << endl;
        for (size_t i = 0; i < trends.size(); i++) {
            const ZigZag& zz = *trends[i].zigzag_1;
            for (size_t j = 0; j < zz.size(); j++) {
                const Zig& point = zz[j];
                file_1 << i + 1 << ","
                         << j + 1 << ","
                         << point.t << ","
                         << utils::get_utc_datetime_string(point.t) << ","
                         << point.p << ","
                         << (point.h ? "true" : "false") << endl;
            }
        }
        file_1.close();
        cout << "Saved zigzag_1 points to " << filename_1 << endl;
    }

    // Save zigzag_2 points
    ofstream file_2(filename_2);
    if (file_2.is_open()) {
        file_2 << "trend_id,point_id,timestamp,datetime,price,is_high" << endl;
        for (size_t i = 0; i < trends.size(); i++) {
            const ZigZag& zz = *trends[i].zigzag_2;
            for (size_t j = 0; j < zz.size(); j++) {
                const Zig& point = zz[j];
                file_2 << i + 1 << ","
                         << j + 1 << ","
                         << point.t << ","
                         << utils::get_utc_datetime_string(point.t) << ","
                         << point.p << ","
                         << (point.h ? "true" : "false") << endl;
            }
        }
        file_2.close();
        cout << "Saved zigzag_2 points to " << filename_2 << endl;
    }
}

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

    // Track last timestamps for detecting completed candles
    size_t last_1m_ts = 0;
    size_t last_1h_ts = 0;

    // Subscribe to published candles
    pubsub.subscribe("candle", [&was_ready, &last_1m_ts, &last_1h_ts](const void* data) {
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

        // Store candles and push to TrendDetector once SymbolInfo is ready
        if (symbolInfo.ready) {
            detector.push_candle(*candle);

            // Store 1s candles
            candles_1s_from_ready.push_back(*candle);

            // Store 1m candles (only when a new 1m candle starts)
            if (symbolInfo.candles_1m.size() > 0) {
                const Candle& current_1m = symbolInfo.candles_1m[symbolInfo.candles_1m.size() - 1];
                if (current_1m.t != last_1m_ts) {
                    if (last_1m_ts != 0 && candles_1m_from_ready.size() > 0) {
                        // Update the last stored candle with completed values
                        candles_1m_from_ready.back() = symbolInfo.candles_1m[symbolInfo.candles_1m.size() - 2];
                    }
                    // Push the new candle
                    candles_1m_from_ready.push_back(current_1m);
                    last_1m_ts = current_1m.t;
                } else if (candles_1m_from_ready.size() > 0) {
                    // Update the current candle
                    candles_1m_from_ready.back() = current_1m;
                }
            }

            // Store 1h candles (only when a new 1h candle starts)
            if (symbolInfo.candles_1h.size() > 0) {
                const Candle& current_1h = symbolInfo.candles_1h[symbolInfo.candles_1h.size() - 1];
                if (current_1h.t != last_1h_ts) {
                    if (last_1h_ts != 0 && candles_1h_from_ready.size() > 0) {
                        // Update the last stored candle with completed values
                        candles_1h_from_ready.back() = symbolInfo.candles_1h[symbolInfo.candles_1h.size() - 2];
                    }
                    // Push the new candle
                    candles_1h_from_ready.push_back(current_1h);
                    last_1h_ts = current_1h.t;
                } else if (candles_1h_from_ready.size() > 0) {
                    // Update the current candle
                    candles_1h_from_ready.back() = current_1h;
                }
            }
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
    size_t trend_tail_count_for_error = config.get_int("trend_tail_count_for_error");

    size_t start_ts = utils::get_timestamp(start_datetime);
    size_t end_ts = utils::get_timestamp(end_datetime);

    cout << "Symbol: " << symbol << endl;
    cout << "Start: " << utils::get_utc_datetime_string(start_ts) << endl;
    cout << "End: " << utils::get_utc_datetime_string(end_ts) << endl;
    cout << "Max error: " << max_error << endl;
    cout << endl;

    detector.max_allowed_error = max_error;
    detector.trend_tail_count_for_error = trend_tail_count_for_error;

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
        cout << "  ZigZag points (1): " << trend.zigzag_1->size() << endl;
        cout << "  ZigZag points (2): " << trend.zigzag_2->size() << endl;
    }

    // Save data to CSV files
    cout << "\n=== Saving Data to Files ===" << endl;
    string files_path = config.files_path;

    // Ensure directory exists
    utils::create_directory(files_path);

    save_candles_to_csv(candles_1s_from_ready, files_path + "/candles_1s.csv", "1s");
    save_candles_to_csv(candles_1m_from_ready, files_path + "/candles_1m.csv", "1m");
    save_candles_to_csv(candles_1h_from_ready, files_path + "/candles_1h.csv", "1h");
    save_trends_to_csv(detector.trends, files_path + "/trends.csv");
    save_zigzag_points_to_csv(detector.trends, files_path + "/zigzag_1.csv", files_path + "/zigzag_2.csv");

    cout << "\nAll files saved to: " << files_path << endl;
}

int main() {
    run_trend_detection_test();
    return 0;
}
