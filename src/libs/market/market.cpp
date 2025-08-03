#include "market.hpp"
#include <iostream>
#include "../core/pubsub/pubsub.hpp"
#include "../trade/trade.hpp"
#include <cstring>


using namespace std;

void Order::finalize() {
    duration = exit_ts - entry_ts;
    profit = (10000 * (exit_price - entry_price) / entry_price) * (direction == OrderDirection::LONG ? 1 : -1);
    net_profit = profit - commision;
}

void Order::finalize(size_t exit_ts, double exit_price, ExitStatus exit_status) {
    this->exit_ts = exit_ts;
    this->exit_price = exit_price;
    this->exit_status = exit_status;
    finalize();
}

char * Order::to_buffer() {
    static char buffer[12 * 8];
    size_t offset = 0;
    // clear buffer
    memset(buffer, 0, sizeof(buffer));

    memcpy(buffer + offset, &id, sizeof(size_t));
    offset += sizeof(size_t);
    memcpy(buffer + offset, &external_id, sizeof(size_t));
    offset += sizeof(size_t);
    memcpy(buffer + offset, &entry_ts, sizeof(size_t));
    offset += sizeof(size_t);
    memcpy(buffer + offset, &exit_ts, sizeof(size_t));
    offset += sizeof(size_t);    
    memcpy(buffer + offset, &entry_price, sizeof(double));
    offset += sizeof(double);
    memcpy(buffer + offset, &exit_price, sizeof(double));
    offset += sizeof(double);
    memcpy(buffer + offset, &sl, sizeof(double));
    offset += sizeof(double);
    memcpy(buffer + offset, &tp, sizeof(double));
    offset += sizeof(double);
    memcpy(buffer + offset, &profit, sizeof(double));
    offset += sizeof(double);
    memcpy(buffer + offset, &net_profit, sizeof(double));
    offset += sizeof(double);
    memcpy(buffer + offset, &duration, sizeof(size_t));
    offset += sizeof(size_t);
    size_t dir = (direction == OrderDirection::LONG) ? 0 : 1;
    memcpy(buffer + offset, &dir, sizeof(size_t));
    return buffer;
}

ostream & operator<<(ostream &os, const Order &order) {
    os << "Order ID: " << order.id << ", External ID: " << order.external_id << ", Request TS: " << order.request_ts
       << ", Entry TS: " << order.entry_ts << ", Exit TS: " << order.exit_ts
       << ", Exit Status: " << static_cast<int>(order.exit_status)
       << ", Direction: " << static_cast<int>(order.direction)
       << ", Entry Price: " << order.entry_price
       << ", Exit Price: " << order.exit_price
       << ", SL: " << order.sl
       << ", TP: " << order.tp
       << ", Profit: " << order.profit
       << ", Duration: " << order.duration;
    return os;
}

Market::Market(const string &name) : name(name) {
    cout << "Market created: " << name << endl;
}

Market * Market::subscribe_to_pubsub() {
    PubSub& pubsub = PubSub::getInstance();
    pubsub.subscribe("trade", [this](void* data) { 
        Trade* trade = static_cast<Trade*>(data);
        this->push(trade->t, trade->p);
    });
    // pubsub.subscribe("trade_finished", [this](void* data) { cout << "Market finished" << endl << report() << endl; });
    return this;
}

Market * Market::set_commision(double commision) {
    this->commision = commision;
    return this;
}

Market * Market::set_price_multiplier_to_handle_orders(double price_multiplier_to_handle_orders) {
    this->price_multiplier_to_handle_orders = price_multiplier_to_handle_orders;
    return this;
}

void Market::push(size_t ts, double price) {
    last_ts = ts;
    last_price = price;
    if (last_handling_price < epsilon) {
        last_handling_price = price;
    }
    if (abs(price - last_handling_price) > price_multiplier_to_handle_orders * last_handling_price) {
        last_handling_price = price;
        handle_pending_orders();
        handle_active_orders();
    }
}

void Market::handle_pending_orders() {
    for (auto it = pending_orders.begin(); it != pending_orders.end();) {
        if (it->cancel_duration > 0 && (last_ts - it->request_ts) >= it->cancel_duration) {
            it = pending_orders.erase(it);
        }
        else if (it->cancel_price_lower > epsilon && last_price <= it->cancel_price_lower) {
            it = pending_orders.erase(it);
        }
        else if (it->cancel_price_upper > epsilon && last_price >= it->cancel_price_upper) {
            it = pending_orders.erase(it);
        }
        else if (it->activate_price_lower > epsilon && last_price <= it->activate_price_lower) {
            it->entry_ts = last_ts;
            it->entry_price = last_price;
            auto next_it = std::next(it);
            active_orders.splice(active_orders.end(), pending_orders, it);
            it = next_it;
        } 
        else if (it->activate_price_upper > epsilon && last_price >= it->activate_price_upper) {
            it->entry_ts = last_ts;
            it->entry_price = last_price;
            auto next_it = std::next(it);
            active_orders.splice(active_orders.end(), pending_orders, it);
            it = next_it;
        } 
        else {
            ++it;
        }
    }
}

void Market::handle_active_orders() {
    for (auto it = active_orders.begin(); it != active_orders.end();) {
        if (it->direction == OrderDirection::LONG && it->sl > epsilon && last_price <= it->sl) {
            it->finalize(last_ts, last_price, ExitStatus::SL);
            auto next_it = std::next(it);
            completed_orders.splice(completed_orders.end(), active_orders, it);
            it = next_it;
        }
        else if (it->direction == OrderDirection::SHORT && it->sl > epsilon && last_price >= it->sl) {
            it->finalize(last_ts, last_price, ExitStatus::SL);
            auto next_it = std::next(it);
            completed_orders.splice(completed_orders.end(), active_orders, it);
            it = next_it;
        }
        else if (it->tp > epsilon && ((it->direction == OrderDirection::LONG && last_price >= it->tp) ||
                               (it->direction == OrderDirection::SHORT && last_price <= it->tp))) {
            it->finalize(last_ts, last_price, ExitStatus::TP);
            auto next_it = std::next(it);
            completed_orders.splice(completed_orders.end(), active_orders, it);
            it = next_it;
        }
        else {
            ++it;
        }

    }
}


Order * Market::market_order(OrderDirection direction) {
    Order *order = &active_orders.emplace_back();
    order->id = ++last_order_id;
    order->direction = direction;
    order->request_ts = last_ts;
    order->entry_ts = last_ts;
    order->entry_price = last_price;
    order->commision = commision;
    return order;
}

Order * Market::pending_order(OrderDirection direction) {
    Order *order = &pending_orders.emplace_back();
    order->id = ++last_order_id;
    order->direction = direction;
    order->request_ts = last_ts;
    order->commision = commision;
    return order;
}

void Market::cancel_order(size_t id) {
    for (auto it = pending_orders.begin(); it != pending_orders.end(); ++it) {
        if (it->id == id) {
            pending_orders.erase(it);
            return;
        }
    }
    for (auto it = active_orders.begin(); it != active_orders.end(); ++it) {
        if (it->id == id) {
            it->finalize(last_ts, last_price, ExitStatus::Cancel);
            completed_orders.push_back(*it);
            active_orders.erase(it);
            return;
        }
    }
}

MarketReport Market::report() const {
    MarketReport report;
    report.total_orders = completed_orders.size() + active_orders.size() + pending_orders.size();
    report.completed_orders = completed_orders.size();
    report.active_orders = active_orders.size();
    report.pending_orders = pending_orders.size();

    for (const auto &order : completed_orders) {
        if (order.exit_status == ExitStatus::SL) {
            report.sl++;
        } else if (order.exit_status == ExitStatus::TP) {
            report.tp++;
        } else if (order.exit_status == ExitStatus::Cancel) {
            report.cancel++;
        }
        if (order.net_profit > 0) {
            report.success++;
            report.avg_duration_success += order.duration;
        } else {
            report.failed++;
            report.avg_duration_failed += order.duration;
        }
        report.total_profit += order.profit;
        report.total_net_profit += order.net_profit;
        report.avg_duration += order.duration;

    }

    report.win_rate = (report.success + report.failed) > 0 ? (static_cast<double>(report.success) / (report.success + report.failed)) * 100 : 0;
    report.avg_duration = report.completed_orders > 0 ? report.avg_duration / report.completed_orders : 0;
    report.avg_duration_success = report.success > 0 ? report.avg_duration_success / report.success : 0;
    report.avg_duration_failed = report.failed > 0 ? report.avg_duration_failed / report.failed : 0;

    return report;
}


ostream & operator<<(ostream &os, const MarketReport &report) {
    os << "Total Orders: " << report.total_orders << ", Completed Orders: " << report.completed_orders
       << ", Active Orders: " << report.active_orders << ", Pending Orders: " << report.pending_orders
       << ", Success: " << report.success << ", Failed: " << report.failed
       << ", SL: " << report.sl << ", TP: " << report.tp
       << ", Cancel: " << report.cancel
       << ", Win Rate: " << report.win_rate << "%"
       << ", Avg Duration: " << report.avg_duration
       << ", Avg Duration Success: " << report.avg_duration_success
       << ", Avg Duration Failed: " << report.avg_duration_failed
       << ", Total Profit: " << report.total_profit
       << ", Total Net Profit: " << report.total_net_profit;
    return os;
}

