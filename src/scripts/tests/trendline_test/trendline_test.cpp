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

    ofstream trade_file;
    ofstream zigzag_file;
    ofstream up_trend_line_file;

    trade_file.open(config.files_path + "trades.bin", ios::out | ios::binary);
    zigzag_file.open(config.files_path + "zigzag.bin", ios::out | ios::binary);
    up_trend_line_file.open(config.files_path + "up_trend_line.bin", ios::out | ios::binary);

    PubSub& pubsub = PubSub::getInstance();

    pubsub.subscribe("trade", [&trade_file](void* data) {
        Trade* trade = static_cast<Trade*>(data);
        trade_file.write((char*)&trade->t, sizeof(trade->t));
        trade_file.write((char*)&trade->p, sizeof(trade->p));
    });

    pubsub.subscribe(up_trend_line->publish_topic + "_zigzag_append", [&zigzag_file, &up_trend_line](void* data) {
        ZigZag * z = up_trend_line->zigzag;
        if (z->size() > 2) {
            auto last_1 = z->end() - 2;
            zigzag_file.write((char*)&last_1->t, sizeof(last_1->t));
            zigzag_file.write((char*)&last_1->p, sizeof(last_1->p));
        }
    });

    pubsub.subscribe(up_trend_line->publish_topic, [&up_trend_line_file](void* data) {
        char* buffer = static_cast<char*>(data);
        // cout << "Received up trend line data." << endl;
        up_trend_line_file.write(buffer, 64); // Write 64 bytes of the trend line data
    });


    trade_reader.pubsub_trades(config.get_timestamp("datetime1"), config.get_timestamp("datetime2"));



    trade_file.close();
    zigzag_file.close();
    up_trend_line_file.close();


    cout << "All finished reading trades." << endl;
}



int main() {
    test1();
    return 0;
}


