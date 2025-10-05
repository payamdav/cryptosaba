#include <string>
#include <iostream>
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/utils//datetime_utils.hpp"
#include "../../../libs/trade/trade.hpp"



using namespace std;


void print_information_of_symbol(const string& symbol) {
    TradeReader trade_reader(symbol);
    size_t count = trade_reader.count;
    Trade first_trade = trade_reader.read_first();
    Trade last_trade = trade_reader.read_last();

    cout << "Symbol: " << symbol << " - ";
    cout << "Total trades: " << count << " - ";
    if (count > 0) {
        cout << "First: " << first_trade.t << " - " << utils::get_utc_datetime_string(first_trade.t) << " - ";
        cout << "Last: " << last_trade.t << " - " << utils::get_utc_datetime_string(last_trade.t) << " - ";
        cout << "Days: " << (last_trade.t - first_trade.t) / (1000 * 60 * 60 * 24);
        cout << endl;
    }
    // cout << "----------------------------------------" << endl;
}

void print_information_of_all_symbols() {
    Config & config = Config::getInstance();
    vector<string> symbols = config.get_csv_strings("symbols");
    for (const auto& symbol : symbols) {
        print_information_of_symbol(symbol);
    }
    cout << "Currrent Selected Symbol: " << config.get("symbol") << endl;
}

int main() {
    print_information_of_all_symbols();
    return 0;
}
