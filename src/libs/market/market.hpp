#pragma once
#include <string>
#include <list>
#include <iostream>


using namespace std;

constexpr double epsilon = 1e-8;

enum class ExitStatus {
    UnCompleted,
    SL,
    TP,
    Cancel
};

enum class OrderDirection {
    LONG,
    SHORT
};


class Order {
    public:
        size_t id = 0;
        size_t request_ts = 0;
        size_t entry_ts = 0;
        size_t exit_ts = 0;
        ExitStatus exit_status = ExitStatus::UnCompleted;
        OrderDirection direction = OrderDirection::LONG;

        double activate_price_upper = 0; // Price to activate the order (for limit orders)
        double activate_price_lower = 0; // Price to activate the order (for limit orders)
        double cancel_price_upper = 0; // Price to cancel the order (for limit orders)
        double cancel_price_lower = 0; // Price to cancel the order (for limit orders)
        size_t cancel_duration = 0; // Duration to cancel the order (for limit orders)

        double entry_price = 0;
        double exit_price = 0;
        double sl = 0; // Stop Loss
        double tp = 0; // Take Profit
        double profit = 0; // Profit in pips
        double net_profit = 0; // Net profit in pips
        size_t duration = 0; // Duration in seconds

        double commision = 0; // Commission in pips

        void finalize();
        void finalize(size_t exit_ts, double exit_price, ExitStatus exit_status);

};

ostream & operator<<(ostream &os, const Order &order);

struct MarketReport {
    size_t total_orders = 0;
    size_t completed_orders = 0;
    size_t active_orders = 0;
    size_t pending_orders = 0;
    size_t success = 0;
    size_t failed = 0;
    size_t sl = 0; // Number of orders closed by Stop Loss
    size_t tp = 0; // Number of orders closed by Take Profit
    size_t cancel = 0; // Number of orders canceled
    double win_rate = 0; // Win rate in percentage
    double avg_duration = 0; // Average duration in seconds
    double avg_duration_success = 0; // Average duration of successful orders in seconds
    double avg_duration_failed = 0; // Average duration of failed orders in seconds
    double total_profit = 0; // Total profit in pips
    double total_net_profit = 0; // Total net profit in pips
};

ostream & operator<<(ostream &os, const MarketReport &report);

class Market {
    private:
        void handle_pending_orders();
        void handle_active_orders();
    public:
        string name;
        size_t last_order_id = 0;
        size_t last_ts = 0; // Last timestamp of the market
        double last_price = 0; // Last price of the market
        double last_handling_price = 0; // Last price of the market for handling orders
        double price_multiplier_to_handle_orders = 0.0001; // Price multiplier to handle orders
        double commision = 0; // Commission in pips
        list<Order> pending_orders;
        list<Order> active_orders;
        list<Order> completed_orders;

        Market(const string &name = "Default Market");
        Market * set_commision(double commision);
        Market * set_price_multiplier_to_handle_orders(double price_multiplier_to_handle_orders);
        void push(size_t ts, double price);
        Order * market_order(OrderDirection direction);
        Order * pending_order(OrderDirection direction);
        void cancel_order(size_t id);
        MarketReport report() const;
        Market * subscribe_to_pubsub();
};
