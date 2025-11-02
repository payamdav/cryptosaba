#include "trend_detector.hpp"
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/utils/datetime_utils.hpp"
#include <iostream>

using namespace std;

void run_trend_detection_test() {
    Config& config = Config::getInstance();

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

    TrendDetector detector;
    detector.max_allowed_error = max_error;
    detector.feed_from_candles(symbol, start_ts, end_ts);

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
        cout << "  R^2: " << trend.r_squared << endl;
        cout << "  Error: " << trend.error << endl;
        cout << "  Candles: " << trend.candles.size() << endl;
    }
}

int main() {
    run_trend_detection_test();
    return 0;
}
