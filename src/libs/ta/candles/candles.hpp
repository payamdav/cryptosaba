#pragma once
#include "../../core/pubsub/pubsub.hpp"
#include "../../trade/trade.hpp"
#include <boost/circular_buffer.hpp>
#include <iostream>
#include <string>
#include <vector>


class Candle {
    public:
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
        Candle(const Trade& trade, size_t sec); // initiate candle by first trade that is part of it
        Candle(const Candle& candle, size_t sec); // initiate candle by first candle that is part of it
        void push(const Trade& trade, size_t sec); // add trade to candle
        void push(const Candle& candle, size_t sec); // add candle to candle ( useful for building higher timeframe candles from 1s timeframe candles )
        bool is_time_in_candle(size_t t, size_t sec);
        bool is_time_before_candle(size_t t);
        bool is_time_next_candle(size_t t, size_t sec);
        Candle filling_candle(size_t sec); // return a new candle that fills the gap for empty time slot
};

std::ostream& operator<<(std::ostream& os, const Candle& candle);

class Candles : public boost::circular_buffer<Candle> {
    private:
        PubSub& pubsub = PubSub::getInstance();
        bool publish_candles_on_update = false;
        std::string topic_candles_on_new_candle;
        
    public:
        size_t sec; // Candle duration in seconds

        Candles(size_t sec=1, size_t retain_count=1000);

        void push(const Candle& candle); // push new candle ( only one second candles are allowed )
        void push(const Trade& trade); // push new trade ( will create new candles as needed )

        Candles * subscribe_to_pubsub_trades();
        Candles * subscribe_to_pubsub_candles(); // subscribe to 1 second candles
        Candles * build_from_1s_candles(const Candles& candles_1s);
};

class CandlesVector : public std::vector<Candle> {
    public:
        size_t sec; // Candle duration in seconds

        CandlesVector(size_t sec=1) : std::vector<Candle>() {
            this->sec = sec;
        }

        void push(const Candle& candle); // push new candle ( only one second candles are allowed )
        void push(const Trade& trade); // push new trade ( will create new candles as needed )
        void build_from_trade_vector(const std::vector<Trade>& trades);
        void write_to_binary_file(const std::string& file_path_name);
};
