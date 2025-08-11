#pragma once
#include <vector>
#include <deque>
#include "../../../core/pubsub/pubsub.hpp"
#include "../../zigzag/zigzag.hpp"
#include <iostream>
#include "../../../trade/trade.hpp"
#include "../../../trade/tradecache.hpp"


using namespace std;

class UpTrendLineZigZag {
    public:
        PubSub & pubsub = PubSub::getInstance();

        deque<Zig> lows;
        deque<Zig> highs;
        bool exists = false;
        size_t serial_number = 0;
        size_t start_t = 0;
        double start_p = 0.0;
        double slope = 0.0;
        double min_intercept = 0.0;
        double max_intercept = 0.0;
        
        void clear(string reason = "");
        void publish_current();

    public:
        ZigZag * zigzag;

        double delta;
        double min_threshold;
        string publish_topic = "up_trend_line_zigzag";

        UpTrendLineZigZag(double delta = 0.0050, double min_threshold = 0.0001);
        ~UpTrendLineZigZag();
        void check();
        bool is_point_inside(size_t t, double p) const;
        bool is_point_above(size_t t, double p) const;
        bool is_point_below(size_t t, double p) const;


};


class TrendLineZigZagEnhancedTradeRegression {
    public:
        PubSub & pubsub = PubSub::getInstance();
        TradeCacheSimple & trade_cache = TradeCacheSimple::getInstance();

        deque<ZigTradeCache> lows;
        deque<ZigTradeCache> highs;
        bool exists = false;
        bool is_up = false;
        size_t serial_number = 0;
        size_t start_t = 0;
        double start_p = 0.0;
        double slope = 0.0;
        double min_intercept = 0.0;
        double max_intercept = 0.0;
        
        void clear(string reason = "");
        void publish_current();

    public:
        ZigZagEnhanced * zigzag;

        double dh;
        double dl;
        double threshold;
        string publish_topic = "trend_line_zigzag_trade_regression";

        TrendLineZigZagEnhancedTradeRegression(double delta, double threshold);
        TrendLineZigZagEnhancedTradeRegression(double delta_h, double delta_l, double threshold);
        ~TrendLineZigZagEnhancedTradeRegression();

        void check();
        void check_uptrend();
        void check_downtrend();

        bool is_point_inside(size_t t, double p) const;
        bool is_point_above(size_t t, double p) const;
        bool is_point_below(size_t t, double p) const;


};


class TrendLineUp2Points{
    public:
        PubSub & pubsub = PubSub::getInstance();
        TradeCacheSimple & trade_cache = TradeCacheSimple::getInstance();

        bool exists = false;
        size_t serial_number = 0;
        size_t start_t = 0;
        double start_p = 0.0;
        double slope = 0.0;
        double min_intercept = 0.0;
        double max_intercept = 0.0;
        
        void clear();
        void publish_current();

    public:
        ZigZagEnhanced * zigzag;

        double dh;
        double dl;
        double threshold;
        string publish_topic = "trend_line_zigzag_trade_regression";

        TrendLineUp2Points(double delta, double threshold);
        TrendLineUp2Points(double delta_h, double delta_l, double threshold);
        ~TrendLineUp2Points();

        void check();

        bool is_point_inside(size_t t, double p) const;
        bool is_point_above(size_t t, double p) const;
        bool is_point_below(size_t t, double p) const;


};

