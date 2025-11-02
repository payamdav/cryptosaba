#pragma once
#include "../../core/pubsub/pubsub.hpp"
#include "../../trade/trade.hpp"
#include <boost/circular_buffer.hpp>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>


class Candle {
    public:
        size_t timeframe = 1; // timeframe in seconds
        size_t t=0;
        double o=0;
        double h=0;
        double l=0;
        double c=0;
        double vwap=0;
        size_t n=0;
        double v=0;
        double vs=0;
        double vb=0;
        double q=0;
        double qs=0;
        double qb=0;

        Candle() = default;
        Candle(size_t timeframe=1);
        Candle(const Trade& trade, size_t timeframe=1); // initiate candle by first trade that is part of it
        Candle(const Candle& candle, size_t timeframe); // initiate candle by first candle that is part of it
        Candle(const char* buffer); // initiate candle from binary buffer (buffer must be at least 112 bytes)
        void push(const Trade& trade); // add trade to candle
        void push(const Candle& candle); // add candle to candle ( useful for building higher timeframe candles from 1s timeframe candles )

        size_t candle_end_time() const;
        bool is_time_in_candle(size_t t);
        bool is_time_before_candle(size_t t);
        bool is_time_next_candle(size_t t);
        Candle filling_candle(); // return a new candle that fills the gap for empty time slot

        void to_buffer(char* buffer) const; // write candle to binary buffer (buffer must be at least 112 bytes)
        static constexpr size_t buffer_size() { return 3 * sizeof(size_t) + 11 * sizeof(double); } // 112 bytes

};

std::ostream& operator<<(std::ostream& os, const Candle& candle);

// Candle builder and writer from trades

class CandleWriter {
    public:
        std::string symbol;
        size_t timeframe; // Candle duration in seconds
        string file_path_name;
        ofstream file_stream;
        Candle candle;
        size_t last_trade_time = 0;
        char buffer[Candle::buffer_size()];
 
        CandleWriter(string symbol, size_t timeframe=1);
        ~CandleWriter();
        void write_current_candle_to_file();
        void push(const Trade& trade); // push new trade ( will create new candles as needed )        
};

string candle_file_path_name(string symbol, size_t timeframe=1);

class CandleReader {
    public:
        string file_path_name;
        ifstream file_stream;
        long long index = 0;
        Candle current_candle;
        long long start = 0;
        long long end = 0;
        size_t sizeof_candle = Candle::buffer_size();
        size_t size = 0;
        size_t first_ts = 0;
        size_t last_ts = 0;
        size_t timeframe = 0;

        CandleReader(string symbol, size_t timeframe=1);
        ~CandleReader();

        void open();
        void close();

        void go(size_t index=0);
        void go_to_timestamp(size_t timestamp);
        bool read_next(Candle& candle);
        bool read_next();
        bool read(Candle& candle, size_t index);
        void set_start(size_t start_index);
        void set_end(size_t end_index);
        size_t ts_to_index(size_t timestamp);
        void publish(size_t start_ts=0, size_t end_ts=0);

};


// candle series




class Candles : public boost::circular_buffer<Candle> {
    private:
        PubSub& pubsub = PubSub::getInstance();
        bool publish_candles_on_update = false;
        std::string topic_candles_on_new_candle;
        
    public:
        size_t timeframe; // Candle duration in seconds

        Candles(size_t timeframe=1, size_t retain_count=1000);

        void push(const Candle& candle); // push new candle ( only one second candles are allowed )
        void push(const Trade& trade); // push new trade ( will create new candles as needed )

        Candles * subscribe_to_pubsub_trades();
        Candles * subscribe_to_pubsub_candles(); // subscribe to 1 second candles
        Candles * build_from_1s_candles(const Candles& candles_1s);
};

class CandlesVector : public std::vector<Candle> {
    public:
        size_t timeframe; // Candle duration in seconds

        CandlesVector(size_t timeframe=1);
        CandlesVector(string symbol, size_t start_ts = 0, size_t end_ts = 0);

        void push(const Candle& candle); // push new candle ( only one second candles are allowed )
        void push(const Trade& trade); // push new trade ( will create new candles as needed )
        void build_from_trade_vector(const std::vector<Trade>& trades);
        void write_to_binary_file(const std::string& file_path_name, string mode="full");
        void report_candles_integrity(const std::string& file_path_name, size_t start_ts=0, size_t end_ts=0);
        void read_from_binary_file(const std::string& symbol, size_t start_ts=0, size_t end_ts=0);
};
