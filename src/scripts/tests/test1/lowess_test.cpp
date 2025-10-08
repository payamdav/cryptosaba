#include <cstddef>
#include <string>
#include <iostream>
#include <vector>
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/utils//datetime_utils.hpp"
#include "../../../libs/trade/trade.hpp"
#include "../../../libs/utils/random_utils.hpp"
#include "../../../libs/ta/candles/candles.hpp"
#include "../../../libs/ta/lowess/lowess.hpp"



using namespace std;


void lowess1(string symbol, size_t ts, size_t half_neighbors_count = 100, size_t index = 200) {
    TradeReader trade_reader(symbol);
    vector<Trade> trades = trade_reader.read_by_ts_to_vector(ts - (24*60*60*1000), ts);  // load last 24 hours trades
    cout << "Loaded " << trades.size() << " trades for symbol " << symbol << " at " << ts << " from " << utils::get_utc_datetime_string(ts - (24*60*60*1000)) << " to " << utils::get_utc_datetime_string(ts) << endl;
    write_trades_to_bin_file_price_ts(Config::getInstance().files_path + "trades.bin", trades);

    CandlesVector candles_1s(1);
    candles_1s.build_from_trade_vector(trades);
    candles_1s.write_to_binary_file(Config::getInstance().files_path + "candles_1s.bin");
    cout << "Built " << candles_1s.size() << " 1 second candles for symbol " << symbol << " at " << ts << endl;

    CandlesVector candles_60s(60);
    candles_60s.build_from_trade_vector(trades);
    candles_60s.write_to_binary_file(Config::getInstance().files_path + "candles_60s.bin");
    cout << "Built " << candles_60s.size() << " 60 second candles for symbol " << symbol << " at " << ts << endl;

    CandlesVector candles_1h(3600);
    candles_1h.build_from_trade_vector(trades);
    candles_1h.write_to_binary_file(Config::getInstance().files_path + "candles_1h.bin");
    cout << "Built " << candles_1h.size() << " 3600 second candles for symbol " << symbol << " at " << ts << endl;

    LowessResult lowess_result = lowess(candles_1s, half_neighbors_count, index);
    cout << "Lowess result: " << lowess_result.ts << " " << lowess_result.y << " " << lowess_result.w << endl;

    vector<LowessResult> lowess_results = lowess(candles_1s, 2*3600);
    save_lowess_results_to_binary_file(Config::getInstance().files_path + "lowess_1s.bin", lowess_results);

    vector<LowessResult> lowess_results_60s = lowess(candles_60s, 120);
    save_lowess_results_to_binary_file(Config::getInstance().files_path + "lowess_60s.bin", lowess_results_60s);

    vector<LowessResult> lowess_results_1h = lowess(candles_1h, 2);
    save_lowess_results_to_binary_file(Config::getInstance().files_path + "lowess_1h.bin", lowess_results_1h);
}

int main() {
    Config & config = Config::getInstance();
    string symbol = config.get("symbol");
    size_t ts = 1742498333997;
    // size_t ts = utils::random_size_t(config.get_size_t("min_safe_ts") + (24*60*60*1000), config.get_size_t("max_safe_ts"));
    lowess1(symbol, ts, 1000, 200);
    return 0;
}


