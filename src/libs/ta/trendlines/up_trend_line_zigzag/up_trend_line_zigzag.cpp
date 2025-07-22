#include "up_trend_line_zigzag.hpp"
#include <string>



UpTrendLineZigZag::UpTrendLineZigZag(double delta, double min_threshold)
    : delta(delta), min_threshold(min_threshold) {
    zigzag = new ZigZag(delta);
    string zigzag_update_topic = "up_trend_line_zigzag_update_" + std::to_string(delta) + "_" + std::to_string(min_threshold);
    zigzag->set_publish_updates(zigzag_update_topic)->subscribe_to_pubsub();
    pubsub.subscribe(zigzag_update_topic, [this](void* data) { this->check(); });
}

void UpTrendLineZigZag::check() {
    if (zigzag->size() < 5) return; // Not enough data to form a trend line
    auto it = zigzag->rbegin();
    if (this->exists && it->p < lows.back().p ) this->clear("Broken trend line: new low below last low");
    if (!(it->h)) return; // Last point must be a high to form an up trend line
    if (this->exists && (it+1)->t == lows.back().t) return; // same trend line as the last one
    if (this->exists) clear("New low detected, checking for trend line");
    while( it != zigzag->rend() - 2 || it->p <= (it+2)->p) {
        if (it->h) highs.push_front(*it);
        else lows.push_front(*it);
        ++it;
    }
    if (lows.size() < 2) {
        this->clear(); // Not enough lows to form a trend line
        return;
    }

    // calculating trend line
    cout << "Calculating UpTrendLineZigZag with delta: " << delta << " and min_threshold: " << min_threshold << " - count lows: " << lows.size() << endl;

    start_t = lows.front().t;
    // calculate slope of lines connecting lows
    double n = static_cast<double>(lows.size());
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;

    for (size_t i = 0; i < n; ++i) {
        double t = (static_cast<double>(lows[i].t) - start_t) / 1000.0; // convert to seconds
        sum_x += t;
        sum_y += lows[i].p;
        sum_xy += t * lows[i].p;
        sum_x2 += t * t;
    }

    slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
    cout << "Slope: " << slope << endl;

    // check maximum and mimimum intercept of the trend line that pass through lows
    min_intercept = std::numeric_limits<double>::max();
    max_intercept = -1 * min_intercept;
    for (size_t i = 0; i < n; ++i) {
        double t = (static_cast<double>(lows[i].t) - start_t) / 1000.0; // convert to seconds
        double intercept = lows[i].p - slope * t; // y = mx + b => b = y - mx
        if (intercept < min_intercept) min_intercept = intercept;
        if (intercept > max_intercept) max_intercept = intercept;
    }

    double vertical_distance = max_intercept - min_intercept;
    if (vertical_distance < min_threshold * lows.front().p) {
        min_intercept -= ((min_threshold * lows.front().p) - vertical_distance) / 2.0; // adjust intercept to meet the minimum threshold
        max_intercept += ((min_threshold * lows.front().p) - vertical_distance) / 2.0;
    }
    cout << "Min Intercept: " << min_intercept << ", Max Intercept: " << max_intercept << endl;
    exists = true;

    if (false) {
        // If the trend line is invalid, clear it
        this->clear("");
    }

}

void UpTrendLineZigZag::clear(string reason) {
    lows.clear();
    highs.clear();
    exists = false;
    start_t = 0;
    slope = 0.0;
    min_intercept = 0.0;
    max_intercept = 0.0;
    if (!reason.empty()) {
        cout << "Clearing UpTrendLineZigZag: " << reason << endl;
    }
}