#include <string>
#include <iostream>
#include <fstream>
// #include "../../../libs/utils/timer.hpp"
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/core/pubsub/pubsub.hpp"
#include "../../../libs/trade/trade.hpp"
#include "../../../libs/ta/candles/candles.hpp"
#include "../../../libs/ta/zigzag/zigzag.hpp"
#include "../../../libs/ta/trendlines/up_trend_line_zigzag/up_trend_line_zigzag.hpp"


using namespace std;

void test1() {
    Config & config = Config::getInstance();
    string symbol = config.get("symbol");
    TradeReader trade_reader(symbol);
    UpTrendLineZigZag * up_trend_line = new UpTrendLineZigZag(config.get_double("trendline_zigzag"), config.get_double("trendline_threshould"));

    trade_reader.pubsub_trades(config.get_timestamp("datetime1"), config.get_timestamp("datetime2"));

    cout << "All finished reading trades." << endl;
}



int main() {
    test1();
    return 0;
}


