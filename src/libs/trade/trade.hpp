#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

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
};