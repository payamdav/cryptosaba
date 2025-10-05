#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>


using namespace std;

class Trade {
    public:
        double p;
        double v;
        double q;
        size_t t;
        bool is_buyer_maker;
};

ostream& operator<<(ostream& os, const Trade& trade);


class TradeReader {
    public:
        string symbol;
        ifstream trade_data;
        size_t count; // Number of trades in binary file
        
    public:
        TradeReader(string symbol);
        ~TradeReader();
        void open();
        void close();
        void set_file_cursor(size_t pos=0);
        void next(Trade &trade);
        bool read_trade(size_t index, Trade &trade);
        Trade read_trade(size_t index);
        Trade read_first();
        Trade read_last();
        size_t search(size_t t);
        void pubsub_trades(size_t t1=0, size_t t2=0);
        vector<Trade> read_by_ts_to_vector(size_t t1=0, size_t t2=0);

};

// utilities related to trades

void write_trades_to_bin_file(string file_path_name, const vector<Trade>& trades);
void write_trades_to_bin_file_price_ts(string file_path_name, const vector<Trade>& trades);
