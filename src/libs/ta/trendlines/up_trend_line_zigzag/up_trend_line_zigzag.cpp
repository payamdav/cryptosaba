#include "up_trend_line_zigzag.hpp"
#include <string>



UpTrendLineZigZag::UpTrendLineZigZag(double delta, double min_threshold)
    : delta(delta), min_threshold(min_threshold) {
    publish_topic = "up_trend_line_zigzag_" + std::to_string(delta) + "_" + std::to_string(min_threshold);
    zigzag = new ZigZag(delta);
    string zigzag_update_topic = "up_trend_line_zigzag_update_" + std::to_string(delta) + "_" + std::to_string(min_threshold);
    zigzag->set_publish_updates(zigzag_update_topic)->set_publish_appends(publish_topic + "_zigzag_append")->subscribe_to_pubsub();
    pubsub.subscribe(zigzag_update_topic, [this](void* data) { this->check(); });
}

UpTrendLineZigZag::~UpTrendLineZigZag() {
    delete zigzag;
}

void UpTrendLineZigZag::check() {
    if (zigzag->size() < 5) return; // Not enough data to form a trend line
    auto it = zigzag->rbegin();
    if (this->exists && it->p < lows.back().p ) {this->clear("Broken trend line: new low below last low"); return;} // Broken trend line, new low below last low
    if (!(it->h)) return; // Last point must be a high to form an up trend line
    if (this->exists && (it+1)->t == lows.back().t) return; // same trend line as the last one
    clear(); // Here the last point is high, the latest low in zigzag is not the same as the last low in trend line, so we need to check for a new trend line
    while( it != zigzag->rend() - 2 && it->p > (it+2)->p) {
        if (it->h) highs.push_front(*it);
        else lows.push_front(*it);
        ++it;
    }
    if (it->h) {
        highs.push_front(*it);
        lows.push_front(*(it+1)); // The last point is a high, so we need to add the next point as a low
   
    }
    else {
        lows.push_front(*it);
        highs.push_front(*(it+1)); // The last point is a low, so we need to add the next point as a high
    }

    if (lows.size() < 2) {
        this->clear(); // Not enough lows to form a trend line
        return;
    }

    // calculating trend line

    start_t = lows.front().t;
    start_p = lows.front().p;
    // calculate slope of lines connecting lows
    double n = static_cast<double>(lows.size());
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;

    for (size_t i = 0; i < n; ++i) {
        double t = (static_cast<double>(lows[i].t) - start_t) / 1000.0; // convert to seconds
        double p = lows[i].p - start_p; // relative price to the first low
        sum_x += t;
        sum_y += p;
        sum_xy += t * p;
        sum_x2 += t * t;
    }

    slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);

    // check maximum and mimimum intercept of the trend line that pass through lows
    min_intercept = std::numeric_limits<double>::max();
    max_intercept = -1 * min_intercept;
    for (size_t i = 0; i < n; ++i) {
        double t = (static_cast<double>(lows[i].t) - start_t) / 1000.0; // convert to seconds
        double p = lows[i].p - start_p; // relative price to the first low
        double intercept = p - slope * t; // y = mx + b => b = y - mx
        if (intercept < min_intercept) min_intercept = intercept;
        if (intercept > max_intercept) max_intercept = intercept;
    }

    double vertical_distance = max_intercept - min_intercept;
    if (vertical_distance < min_threshold * lows.front().p) {
        min_intercept -= ((min_threshold * lows.front().p) - vertical_distance) / 2.0; // adjust intercept to meet the minimum threshold
        max_intercept += ((min_threshold * lows.front().p) - vertical_distance) / 2.0;
    }
    exists = true;
    serial_number++;

    cout << "Calculating UpTrendLineZigZag with delta: " << delta << " and min_threshold: " << min_threshold << " - count lows: " << lows.size() << " - slope: " << slope << " - min_intercept: " << min_intercept << " - max_intercept: " << max_intercept << endl;
    if (slope < 0) {
        for (const auto& low : lows) {
            cout << "Low: t=" << low.t << ", p=" << low.p << ", h=" << low.h << endl;
        }
        cout << "------------------------------------------------------" << endl;
        cout << "printing zigzag" << endl;
        for (const auto& point : *zigzag) {
            cout << "ZigZag: t=" << point.t << ", p=" << point.p << ", h=" << point.h << endl;
        }
        cout << "Trend line is invalid, slope is negative." << endl;
    }

    if (false) {
        // If the trend line is invalid, clear it
        this->clear("");
    }

}

void UpTrendLineZigZag::clear(string reason) {
    if (exists) this->publish_current();

    lows.clear();
    highs.clear();
    exists = false;
    start_t = 0;
    start_p = 0.0;
    slope = 0.0;
    min_intercept = 0.0;
    max_intercept = 0.0;
    if (!reason.empty()) cout << "Clearing UpTrendLineZigZag: " << reason << endl;
}

void UpTrendLineZigZag::publish_current() {
    if (!exists) return;
    size_t t_start = start_t;
    size_t t_end = lows.back().t;
    double p_start_min = start_p + min_intercept;
    double p_start_max = start_p + max_intercept;
    double p_end_min = slope * (static_cast<double>(t_end - t_start) / 1000.0) + p_start_min;
    double p_end_max = slope * (static_cast<double>(t_end - t_start) / 1000.0) + p_start_max;
    size_t count = lows.size();
    double slope_value = this->slope;
    size_t serial_num = this->serial_number;

    // publish as binary data - 72 bytes total
    char buffer[72];
    size_t offset = 0;

    // Write size_t t_start (8 bytes)
    memcpy(buffer + offset, &t_start, sizeof(size_t));
    offset += sizeof(size_t);

    // Write size_t t_end (8 bytes)
    memcpy(buffer + offset, &t_end, sizeof(size_t));
    offset += sizeof(size_t);

    // Write double p_start_min (8 bytes)
    memcpy(buffer + offset, &p_start_min, sizeof(double));
    offset += sizeof(double);

    // Write double p_start_max (8 bytes)
    memcpy(buffer + offset, &p_start_max, sizeof(double));
    offset += sizeof(double);

    // Write double p_end_min (8 bytes)
    memcpy(buffer + offset, &p_end_min, sizeof(double));
    offset += sizeof(double);

    // Write double p_end_max (8 bytes)
    memcpy(buffer + offset, &p_end_max, sizeof(double));
    offset += sizeof(double);

    // Write size_t count (8 bytes)
    memcpy(buffer + offset, &count, sizeof(size_t));
    offset += sizeof(size_t);

    // Write double slope (8 bytes)
    memcpy(buffer + offset, &slope_value, sizeof(double));
    offset += sizeof(double);

    // write serial number (8 bytes)
    memcpy(buffer + offset, &serial_num, sizeof(size_t));
    
    pubsub.publish(publish_topic, buffer);
}


bool UpTrendLineZigZag::is_point_below(size_t t, double p) const {
    if (!exists) return false;
    double relative_t = static_cast<double>(t - start_t) / 1000.0; // convert to seconds
    double expected_p = slope * relative_t + start_p + min_intercept; // y = mx + b
    return (p < expected_p);
}

bool UpTrendLineZigZag::is_point_above(size_t t, double p) const {
    if (!exists) return false;
    double relative_t = static_cast<double>(t - start_t) / 1000.0; // convert to seconds
    double expected_p = slope * relative_t + start_p + max_intercept; // y = mx + b
    return (p > expected_p);
}

bool UpTrendLineZigZag::is_point_inside(size_t t, double p) const {
    if (!exists) return false;
    double relative_t = static_cast<double>(t - start_t) / 1000.0; // convert to seconds
    double expected_p_min = slope * relative_t + start_p + min_intercept; // y = mx + b
    double expected_p_max = slope * relative_t + start_p + max_intercept; // y = mx + b
    return (p >= expected_p_min && p <= expected_p_max);
}




// Implementation of TrendLineZigZagEnhancedTradeRegression

TrendLineZigZagEnhancedTradeRegression::TrendLineZigZagEnhancedTradeRegression(double delta_h, double delta_l, double threshold)
    : dh(delta_h), dl(delta_l), threshold(threshold) {
    zigzag = new ZigZagEnhanced(delta_h, delta_l);
    string zigzag_update_topic = "trend_line_zigzag_trade_regression_update_" + std::to_string(dh) + "_" + std::to_string(dl) + "_" + std::to_string(threshold);
    zigzag->set_publish_updates(zigzag_update_topic)->set_publish_appends(publish_topic + "_zigzag_append")->subscribe_to_pubsub();
    pubsub.subscribe(zigzag_update_topic, [this](void* data) { this->check();});
}

TrendLineZigZagEnhancedTradeRegression::TrendLineZigZagEnhancedTradeRegression(double delta, double threshold)
    : TrendLineZigZagEnhancedTradeRegression(delta, delta, threshold) {

}

TrendLineZigZagEnhancedTradeRegression::~TrendLineZigZagEnhancedTradeRegression() {
    delete zigzag;
}

void TrendLineZigZagEnhancedTradeRegression::check() {
    if (zigzag->size() < 5) return; // Not enough data to form a trend line

    if (this->exists && this->is_up && zigzag->back().p < lows.back().p) {this->clear("Broken trend line: new low below last low"); return;} // Broken trend line, new low below last low
    if (this->exists && !(this->is_up) && zigzag->back().p > highs.back().p) {this->clear("Broken trend line: new high above last high"); return;} // Broken trend line, new high above last high

    if (this->exists && this->is_up) check_uptrend();
    if (this->exists && !(this->is_up)) check_downtrend();
    if (!(this->exists)) check_uptrend(); // If trend line does not exist, check for uptrend first
    if (!(this->exists)) check_downtrend(); // If trend line does not exist, check for downtrend
}

void TrendLineZigZagEnhancedTradeRegression::check_uptrend() {
    auto it = zigzag->rbegin();
    if (!(it->h)) return; // Last point must be a high to form an up trend line
    if (this->exists && this->is_up && (it+1)->t == lows.back().t) return; // same trend line as the last one
    clear(); // Here the last point is high, the latest low in zigzag is not
    while( it != zigzag->rend() - 2 && it->p > (it+2)->p) {
        if (it->h) highs.push_front(*it);
        else lows.push_front(*it);
        ++it;
    }
    if (it->h) {
        highs.push_front(*it);
        lows.push_front(*(it+1)); // The last point is a high, so we need to add the next point as a low
    }
    else {
        lows.push_front(*it);
        highs.push_front(*(it+1)); // The last point is a low, so we need to add the next point as a high
    }

    if (lows.size() < 2) {
        this->clear(); // Not enough lows to form a trend line
        return;
    }
    // calculating trend line
    start_t = lows.front().t;
    start_p = lows.front().p;

    // calculate slope of trades from first low to last trade
    auto trade_start_it = lows.front().it;
    double n = static_cast<double>(distance(trade_start_it, this->trade_cache.last_trade_iterator()));
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double t = (static_cast<double>((trade_start_it + i)->t - start_t) / 1000.0); // convert to seconds
        double p = (trade_start_it + i)->p - start_p; // relative price to the first low
        sum_x += t;
        sum_y += p;
        sum_xy += t * p;
        sum_x2 += t * t;
    }
    slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
    if (slope < 0) {
        this->clear("Invalid trend line: slope is negative");
        return;
    }
    // check maximum and mimimum intercept of the trend line that pass through lows
    min_intercept = std::numeric_limits<double>::max();
    max_intercept = -1 * min_intercept;
    for (size_t i = 0; i < lows.size(); ++i) {
        double t = (static_cast<double>(lows[i].t) - start_t) / 1000.0; // convert to seconds
        double p = lows[i].p - start_p; // relative price to the first low
        double intercept = p - slope * t; // y = mx + b => b = y - mx
        if (intercept < min_intercept) min_intercept = intercept;
        if (intercept > max_intercept) max_intercept = intercept;
    }
    double vertical_distance = max_intercept - min_intercept; // just for information
    max_intercept = min_intercept * (1 + threshold); // set max_intercept to be at least threshold above min_intercept

    exists = true;
    is_up = true;
    serial_number++;
    cout << "Calculating TrendLineZigZagEnhancedTradeRegression with delta_h: " << dh << ", delta_l: " << dl << ", threshold: " << threshold
         << " - count lows: " << lows.size() << " - slope: " << slope
         << " - min_intercept: " << min_intercept << " - max_intercept: " << max_intercept << endl;
}


void TrendLineZigZagEnhancedTradeRegression::check_downtrend() {
    auto it = zigzag->rbegin();
    if (it->h) return; // Last point must be a low to form a down trend line
    if (this->exists && !(this->is_up) && (it+1)->t == highs.back().t) return; // same trend line as the last one
    clear(); // Here the last point is low, the latest high in zigzag is not
    while( it != zigzag->rend() - 2 && it->p < (it+2)->p) {
        if (it->h) highs.push_front(*it);
        else lows.push_front(*it);
        ++it;
    }
    if (it->h) {
        highs.push_front(*it);
        lows.push_front(*(it+1)); // The last point is a low, so we need to add the next point as a high
    }
    else {
        lows.push_front(*it);
        highs.push_front(*(it+1)); // The last point is a high, so we need to add the next point as a low
    }
    if (highs.size() < 2) {
        this->clear(); // Not enough highs to form a trend line
        return;
    }
    // calculating trend line
    start_t = highs.front().t;
    start_p = highs.front().p;
    // calculate slope of trades from first high to last trade
    auto trade_start_it = highs.front().it;
    double n = static_cast<double>(distance(trade_start_it, this->trade_cache.last_trade_iterator()));
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double t = (static_cast<double>((trade_start_it + i)->t - start_t) / 1000.0); // convert to seconds
        double p = (trade_start_it + i)->p - start_p; // relative price to the first high
        sum_x += t;
        sum_y += p;
        sum_xy += t * p;
        sum_x2 += t * t;
    }
    slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
    if (slope > 0) {
        this->clear("Invalid trend line: slope is positive");
        return;
    }
    // check maximum and mimimum intercept of the trend line that pass through highs
    min_intercept = std::numeric_limits<double>::max();
    max_intercept = -1 * min_intercept;
    for (size_t i = 0; i < highs.size(); ++i) {
        double t = (static_cast<double>(highs[i].t) - start_t) / 1000.0; // convert to seconds
        double p = highs[i].p - start_p; // relative price to the first high
        double intercept = p - slope * t; // y = mx + b => b = y - mx
        if (intercept < min_intercept) min_intercept = intercept;
        if (intercept > max_intercept) max_intercept = intercept;
    }
    double vertical_distance = max_intercept - min_intercept; // just for information
    min_intercept = max_intercept * (1 - threshold); // set min_intercept to be at least threshold below max_intercept
    exists = true;
    is_up = false;
    serial_number++;
    cout << "Calculating TrendLineZigZagEnhancedTradeRegression with delta_h: " << dh << ", delta_l: " << dl << ", threshold: " << threshold
         << " - count highs: " << highs.size() << " - slope: " << slope
         << " - min_intercept: " << min_intercept << " - max_intercept: " << max_intercept << endl;
}

void TrendLineZigZagEnhancedTradeRegression::clear(string reason) {
    if (exists) this->publish_current();

    lows.clear();
    highs.clear();
    exists = false;
    is_up = false;
    start_t = 0;
    start_p = 0.0;
    slope = 0.0;
    min_intercept = 0.0;
    max_intercept = 0.0;
    if (!reason.empty()) cout << "Clearing TrendLineZigZagEnhancedTradeRegression: " << reason << endl;
}

void TrendLineZigZagEnhancedTradeRegression::publish_current() {
    if (!exists) return;
    size_t t_start = start_t;
    size_t t_end = (is_up ? lows.back().t : highs.back().t);
    double p_start_min = start_p + min_intercept;
    double p_start_max = start_p + max_intercept;
    double p_end_min = slope * (static_cast<double>(t_end - t_start) / 1000.0) + p_start_min;
    double p_end_max = slope * (static_cast<double>(t_end - t_start) / 1000.0) + p_start_max;
    size_t count = (is_up ? lows.size() : highs.size());
    double slope_value = this->slope;
    size_t serial_num = this->serial_number;

    // publish as binary data - 72 bytes total
    char buffer[72];
    size_t offset = 0;

    // Write size_t t_start (8 bytes)
    memcpy(buffer + offset, &t_start, sizeof(size_t));
    offset += sizeof(size_t);

    // Write size_t t_end (8 bytes)
    memcpy(buffer + offset, &t_end, sizeof(size_t));
    offset += sizeof(size_t);

    // Write double p_start_min (8 bytes)
    memcpy(buffer + offset, &p_start_min, sizeof(double));
    offset += sizeof(double);

    // Write double p_start_max (8 bytes)
    memcpy(buffer + offset, &p_start_max, sizeof(double));
    offset += sizeof(double);

    // Write double p_end_min (8 bytes)
    memcpy(buffer + offset, &p_end_min, sizeof(double));
    offset += sizeof(double);

    // Write double p_end_max (8 bytes)
    memcpy(buffer + offset, &p_end_max, sizeof(double));
    offset += sizeof(double);

    // Write size_t count (8 bytes)
    memcpy(buffer + offset, &count, sizeof(size_t));
    offset += sizeof(size_t);

    // Write double slope (8 bytes)
    memcpy(buffer + offset, &slope_value, sizeof(double));
    offset += sizeof(double);

    // write serial number (8 bytes)
    memcpy(buffer + offset, &serial_num, sizeof(size_t));

    
    pubsub.publish(publish_topic, buffer);
}

bool TrendLineZigZagEnhancedTradeRegression::is_point_below(size_t t, double p) const {
    if (!exists) return false;
    double relative_t = static_cast<double>(t - start_t) / 1000.0; // convert to seconds
    double expected_p = slope * relative_t + start_p + min_intercept; // y = mx + b
    return (p < expected_p);
}

bool TrendLineZigZagEnhancedTradeRegression::is_point_above(size_t t, double p) const {
    if (!exists) return false;
    double relative_t = static_cast<double>(t - start_t) / 1000.0; // convert to seconds
    double expected_p = slope * relative_t + start_p + max_intercept; // y = mx + b
    return (p > expected_p);
}

bool TrendLineZigZagEnhancedTradeRegression::is_point_inside(size_t t, double p) const {
    if (!exists) return false;
    double relative_t = static_cast<double>(t - start_t) / 1000.0; // convert to seconds
    double expected_p_min = slope * relative_t + start_p + min_intercept; // y = mx + b
    double expected_p_max = slope * relative_t + start_p + max_intercept; // y = mx + b
    return (p >= expected_p_min && p <= expected_p_max);
}

