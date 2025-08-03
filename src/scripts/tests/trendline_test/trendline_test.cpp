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
#include "../../../libs/market/market.hpp"


using namespace std;


void trendline_test() {
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
        up_trend_line_file.write(buffer, 72); // Write 72 bytes of the trend line data
    });


    trade_reader.pubsub_trades(config.get_timestamp("datetime1"), config.get_timestamp("datetime2"));



    trade_file.close();
    zigzag_file.close();
    up_trend_line_file.close();


    std::cout << "All finished reading trades." << endl;
}


void up_trendline_long_test() {
    Config & config = Config::getInstance();
    PubSub& pubsub = PubSub::getInstance();
    string symbol = config.get("symbol");
    TradeReader trade_reader(symbol);
    UpTrendLineZigZag * up_trend_line = new UpTrendLineZigZag(config.get_double("trendline_zigzag"), config.get_double("trendline_threshould"));
    ZigZag * trades_zigzag = (new ZigZag(config.get_double("trades_zigzag"), 10))->set_publish_appends("trades_zigzag_append")->subscribe_to_pubsub();
    Market * market1 = (new Market("Market1"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();

    ofstream trade_file;
    ofstream zigzag_file;
    ofstream trade_zigzag_file;
    ofstream up_trend_line_file;
    ofstream long_entry_points_file;
    ofstream orders_file;


    trade_file.open(config.files_path + "trades.bin", ios::out | ios::binary);
    zigzag_file.open(config.files_path + "zigzag.bin", ios::out | ios::binary);
    trade_zigzag_file.open(config.files_path + "trades_zigzag.bin", ios::out | ios::binary);
    up_trend_line_file.open(config.files_path + "up_trend_line.bin", ios::out | ios::binary);
    long_entry_points_file.open(config.files_path + "long_entry_points.bin", ios::out | ios::binary);
    orders_file.open(config.files_path + "orders.bin", ios::out | ios::binary);


    pubsub.subscribe("trades_zigzag_append", [&trades_zigzag, &up_trend_line, &long_entry_points_file, &trade_zigzag_file, &market1](void* data) {
        static size_t last_serial_number = 0;
        if (trades_zigzag->size() < 2) return;
        auto last = trades_zigzag->end() - 1;
        auto last_1 = trades_zigzag->end() - 2;
        // writing zigzag to file
        trade_zigzag_file.write((char*)&last_1->t, sizeof(last_1->t));
        trade_zigzag_file.write((char*)&last_1->p, sizeof(last_1->p));

         if (last_serial_number == up_trend_line->serial_number) return; // No new trend line

        if (!(last->h)) return; // Only consider highs for up trend line
        // if (!up_trend_line->is_point_above(last->t, last->p)) return; // Only consider points above the trend line
        if (up_trend_line->is_point_inside(last_1->t, last_1->p)) {
            last_serial_number = up_trend_line->serial_number;
            std::cout << "Good for long: t=" << last_1->t << ", p=" << last_1->p << endl;

            // create an order on market1
            Order * order = market1->market_order(OrderDirection::LONG);
            order->external_id = last_serial_number;
            order->sl = up_trend_line->lows.back().p;
            order->tp = order->entry_price + (order->entry_price - order->sl) * 2; // Set TP to 2x SL distance

            long_entry_points_file.write((char*)&order->entry_ts, sizeof(order->entry_ts));
            long_entry_points_file.write((char*)&order->entry_price, sizeof(order->entry_price));

        }
    });


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
        up_trend_line_file.write(buffer, 72); // Write 72 bytes of the trend line data
    });


    trade_reader.pubsub_trades(config.get_timestamp("datetime1"), config.get_timestamp("datetime2"));
    for (auto o : market1->completed_orders) {
        orders_file.write(o.to_buffer(), 12 * 8); // Write 96 bytes of the order data
    }



    trade_file.close();
    zigzag_file.close();
    trade_zigzag_file.close();
    up_trend_line_file.close();
    long_entry_points_file.close();
    orders_file.close();

    MarketReport report = market1->report();
    std::cout << "Market Report: " << endl << report << endl;

    std::cout << "All finished reading trades." << endl;
}



int main() {
    // trendline_test();
    up_trendline_long_test();
    return 0;
}


