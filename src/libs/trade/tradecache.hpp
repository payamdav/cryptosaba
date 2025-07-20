#pragma once
#include <boost/circular_buffer.hpp>
#include "trade.hpp"
#include "../core/pubsub/pubsub.hpp"
#include <vector>


class TradeCache {
    private:
        size_t first_trade_ts = 0;
        size_t last_trade_ts = 0;

        size_t seconds_number=0;
        size_t five_seconds_number=0;
        size_t ten_seconds_number=0;
        size_t thirty_seconds_number=0;
        size_t minutes_number=0;
        size_t five_minutes_number=0;
        size_t ten_minutes_number=0;
        size_t thirty_minutes_number=0;
        size_t hours_number=0;

        PubSub& pubsub = PubSub::getInstance();

        TradeCache(size_t size=10000000);
        TradeCache(const TradeCache&) = delete;
        TradeCache& operator=(const TradeCache&) = delete;
        TradeCache(TradeCache&&) = delete;
        TradeCache& operator=(TradeCache&&) = delete;

    public:
        boost::circular_buffer<Trade> cache;

        boost::circular_buffer<boost::circular_buffer<Trade>::iterator> seconds_index;
        boost::circular_buffer<boost::circular_buffer<Trade>::iterator> five_seconds_index;
        boost::circular_buffer<boost::circular_buffer<Trade>::iterator> ten_seconds_index;
        boost::circular_buffer<boost::circular_buffer<Trade>::iterator> thirty_seconds_index;
        boost::circular_buffer<boost::circular_buffer<Trade>::iterator> minutes_index;
        boost::circular_buffer<boost::circular_buffer<Trade>::iterator> five_minutes_index;
        boost::circular_buffer<boost::circular_buffer<Trade>::iterator> ten_minutes_index;
        boost::circular_buffer<boost::circular_buffer<Trade>::iterator> thirty_minutes_index;
        boost::circular_buffer<boost::circular_buffer<Trade>::iterator> hours_index;

        static TradeCache& getInstance();
        void subscribe_to_pubsub();
        boost::circular_buffer<boost::circular_buffer<Trade>::iterator>& get_index(size_t miliseconds);
        void push(Trade& trade);

        size_t get_full_duration_in_hours();
        size_t get_memory_used_in_mb();
        size_t average_count(boost::circular_buffer<boost::circular_buffer<Trade>::iterator> & buf);
        void print_average_counts();
        void reset();
};


class TradeCache2 {
    private:
        size_t first_trade_ts = 0;
        size_t last_trade_ts = 0;

        PubSub& pubsub = PubSub::getInstance();

        TradeCache2(size_t size=10000000);
        TradeCache2(const TradeCache2&) = delete;
        TradeCache2& operator=(const TradeCache2&) = delete;
        TradeCache2(TradeCache2&&) = delete;
        TradeCache2& operator=(TradeCache2&&) = delete;

        std::vector<size_t> milis;
        std::vector<size_t> sizes;
        std::vector<size_t> current_numbers;
        std::vector<boost::circular_buffer<boost::circular_buffer<Trade>::iterator>> indexes;

    public:
        boost::circular_buffer<Trade> cache;

        static TradeCache2& getInstance();
        static TradeCache2* getInstanceP();
        TradeCache2* add_index(size_t miliseconds, size_t size);
        boost::circular_buffer<boost::circular_buffer<Trade>::iterator>& get_index(size_t miliseconds);
        void push(Trade& trade);

        size_t get_full_duration_in_hours();
        size_t get_memory_used_in_mb();
        size_t average_count(boost::circular_buffer<boost::circular_buffer<Trade>::iterator> & buf);
        void print_average_counts();
};
