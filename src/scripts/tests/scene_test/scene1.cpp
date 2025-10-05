#include <cstddef>
#include <string>
#include <iostream>
#include <vector>
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/utils//datetime_utils.hpp"
#include "../../../libs/trade/trade.hpp"
#include "../../../libs/utils/random_utils.hpp"
#include "../../../libs/ta/candles/candles.hpp"



using namespace std;


void scene1(string symbol, size_t ts) {
    TradeReader trade_reader(symbol);
    vector<Trade> trades = trade_reader.read_by_ts_to_vector(ts - (24*60*60*1000), ts);  // load last 24 hours trades
    cout << "Loaded " << trades.size() << " trades for symbol " << symbol << " at " << ts << " from " << utils::get_utc_datetime_string(ts - (24*60*60*1000)) << " to " << utils::get_utc_datetime_string(ts) << endl;
    write_trades_to_bin_file_price_ts(Config::getInstance().files_path + "trades.bin", trades);
    double vol_sum_trades = 0;
    for (const auto& trade : trades) {
        vol_sum_trades += trade.v;
    }
    cout << "Total volume in trades: " << vol_sum_trades << endl;

    CandlesVector candles_1s(1);
    candles_1s.build_from_trade_vector(trades);
    candles_1s.write_to_binary_file(Config::getInstance().files_path + "candles_1s.bin");
    cout << "Built " << candles_1s.size() << " 1 second candles for symbol " << symbol << " at " << ts << endl;
    double vol_sum_candle_1s = 0;
    for (const auto& candle : candles_1s) {
        vol_sum_candle_1s += candle.v;
    }
    cout << "Total volume in 1 second candles: " << vol_sum_candle_1s << endl;

    CandlesVector candles_60s(60);
    candles_60s.build_from_trade_vector(trades);
    candles_60s.write_to_binary_file(Config::getInstance().files_path + "candles_60s.bin");
    cout << "Built " << candles_60s.size() << " 60 second candles for symbol " << symbol << " at " << ts << endl;
    double vol_sum_candle_60s = 0;
    for (const auto& candle : candles_60s) {
        vol_sum_candle_60s += candle.v;
    }
    cout << "Total volume in 60 second candles: " << vol_sum_candle_60s << endl;

    CandlesVector candles_1h(3600);
    candles_1h.build_from_trade_vector(trades);
    candles_1h.write_to_binary_file(Config::getInstance().files_path + "candles_1h.bin");
    cout << "Built " << candles_1h.size() << " 3600 second candles for symbol " << symbol << " at " << ts << endl;
    double vol_sum_candle_1h = 0;
    for (const auto& candle : candles_1h) {
        vol_sum_candle_1h += candle.v;
    }
    cout << "Total volume in 3600 second candles: " << vol_sum_candle_1h << endl;

}

int main() {
    Config & config = Config::getInstance();
    string symbol = config.get("symbol");
    // size_t ts = 1742498333997;
    size_t ts = utils::random_size_t(config.get_size_t("min_safe_ts") + (24*60*60*1000), config.get_size_t("max_safe_ts"));
    scene1(symbol, ts);
    return 0;
}


