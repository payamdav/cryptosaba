#include "up_trend_line_zigzag.hpp"
#include <string>



UpTrendLineZigZag::UpTrendLineZigZag(double delta, double min_threshold)
    : delta(delta), min_threshold(min_threshold) {
    zigzag = new ZigZag(delta);
    string zigzag_update_topic = "up_trend_line_zigzag_update_" + std::to_string(delta) + "_" + std::to_string(min_threshold);
    zigzag->set_publish_updates(zigzag_update_topic)->subscribe_to_pubsub();
    publish_topic = "up_trend_line_zigzag_" + std::to_string(delta) + "_" + std::to_string(min_threshold);
    pubsub.subscribe(zigzag_update_topic, [this](void* data) { this->check(); });
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
    if (exists) {
        // publish the cleared trend line
        size_t t_start = start_t;
        size_t t_end = lows.back().t;
        double p_start_min = start_p + min_intercept;
        double p_start_max = start_p + max_intercept;
        double p_end_min = slope * (static_cast<double>(t_end - t_start) / 1000.0) + p_start_min;
        double p_end_max = slope * (static_cast<double>(t_end - t_start) / 1000.0) + p_start_max;
        size_t count = lows.size();
        double slope = this->slope;
        // cout << "Publishing cleared UpTrendLineZigZag: t_start=" << t_start << ", t_end=" << t_end
        //      << ", p_start_min=" << p_start_min << ", p_start_max=" << p_start_max
        //      << ", p_end_min=" << p_end_min << ", p_end_max=" << p_end_max
        //      << ", count=" << count << ", slope=" << slope << endl;
        // publish as binary data - 64 bytes total
        char buffer[64];
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
        memcpy(buffer + offset, &slope, sizeof(double));
        
        pubsub.publish(publish_topic, buffer);

    }


    lows.clear();
    highs.clear();
    exists = false;
    start_t = 0;
    start_p = 0.0;
    slope = 0.0;
    min_intercept = 0.0;
    max_intercept = 0.0;
    if (!reason.empty()) {
        cout << "Clearing UpTrendLineZigZag: " << reason << endl;
    }
}