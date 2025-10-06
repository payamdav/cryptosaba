#include <cstddef>
#include <string>
#include <iostream>
#include <cstdlib>
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/utils//datetime_utils.hpp"
#include "../../../libs/trade/trade.hpp"


using namespace std;

double epsilon = 1e-6;


void test_trades_of_symbol(const string& symbol) {
    TradeReader trade_reader(symbol);
    size_t count = trade_reader.count;
    size_t count_small_quantity = 0;
    size_t count_small_volume = 0;
    size_t count_invalid = 0;

    trade_reader.set_file_cursor(0);
    Trade trade;
    for (size_t i = 0; i < count; i++) {
        trade_reader.next(trade);
        if (trade.q <= epsilon) {
            count_small_quantity++;
        }
        if (trade.v <= epsilon) {
            count_small_volume++;
        }
        if (abs((trade.q / trade.v) - trade.p) > epsilon) {
            count_invalid++;
            cout << "Invalid trade - ts: " << trade.t << " - date: " << utils::get_utc_datetime_string(trade.t) << endl;
        }
    }

    cout << "" << symbol << " - ";
    cout << "Count: " << count << " - ";
    cout << "Bad q: " << count_small_quantity << " - ";
    cout << "Bad v: " << count_small_volume << " - ";
    cout << "Invalid: " << count_invalid << endl;

}

void test_trades_of_all_symbols() {
    Config & config = Config::getInstance();
    vector<string> symbols = config.get_csv_strings("symbols");
    for (const auto& symbol : symbols) {
        test_trades_of_symbol(symbol);
    }
}

int main() {
    // test_trades_of_symbol("btcusdt");
    test_trades_of_all_symbols();
    return 0;
}
