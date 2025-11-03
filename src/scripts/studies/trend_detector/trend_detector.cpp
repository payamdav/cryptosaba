#include "trend_detector.hpp"
#include "../../../libs/core/pubsub/pubsub.hpp"
#include <cmath>


SymbolInfo::SymbolInfo() :
    candles_1m(60, 7 * 24 * 60),
    candles_1h(3600, 7 * 24),
    avg_candle_size_1s(7 * 24 * 60 * 60),
    avg_candle_size_1m(7 * 24 * 60),
    avg_candle_size_1h(7 * 24),
    avg_vol_1s(7 * 24 * 60 * 60) {
    // Initialize any other members if necessary
}

void SymbolInfo::push_candle_1s(const Candle& candle) {
    this->candles_1m.push(candle);
    this->candles_1h.push(candle);
    this->avg_candle_size_1s.push(abs(candle.h - candle.l));
    this->avg_vol_1s.push(candle.v);

    // Update latest price and vwap from 1s candle
    this->last_price = candle.c;
    this->last_vwap = candle.vwap;

    // Check if a new 1m candle has completed
    if (candles_1m.size() >= 2) {
        const Candle& current_1m = candles_1m[candles_1m.size() - 1];
        if (current_1m.t != last_candle_1m_ts) {
            // New 1m candle started, so previous one is completed
            const Candle& completed_1m = candles_1m[candles_1m.size() - 2];
            this->avg_candle_size_1m.push(abs(completed_1m.h - completed_1m.l));
            this->last_vwap_1m = completed_1m.vwap;
            this->last_candle_1m_ts = current_1m.t;
        }
    } else if (candles_1m.size() == 1) {
        // First candle
        this->last_candle_1m_ts = candles_1m[0].t;
    }

    // Check if a new 1h candle has completed
    if (candles_1h.size() >= 2) {
        const Candle& current_1h = candles_1h[candles_1h.size() - 1];
        if (current_1h.t != last_candle_1h_ts) {
            // New 1h candle started, so previous one is completed
            const Candle& completed_1h = candles_1h[candles_1h.size() - 2];
            this->avg_candle_size_1h.push(abs(completed_1h.h - completed_1h.l));
            this->last_vwap_1h = completed_1h.vwap;
            this->last_candle_1h_ts = current_1h.t;
        }
    } else if (candles_1h.size() == 1) {
        // First candle
        this->last_candle_1h_ts = candles_1h[0].t;
    }

    // Check if all containers are full and set ready flag
    if (!ready) {
        if (candles_1m.full() &&
            candles_1h.full() &&
            avg_candle_size_1s.buffer.full() &&
            avg_candle_size_1m.buffer.full() &&
            avg_candle_size_1h.buffer.full() &&
            avg_vol_1s.buffer.full()) {
            ready = true;
        }
    }
}

Trend::Trend() {
    // Allocate zigzag instances with specified deltas using unique_ptr
    zigzag_001 = make_unique<ZigZag>(0.0010, 1000);
    zigzag_003 = make_unique<ZigZag>(0.0030, 1000);
}

void TrendDetector::push_candle(const Candle& candle) {
    if (trends.empty()) {
        this->push_into_new_trend(candle);
    } else {
        // Logic to determine if candle continues current trend or starts new trend
        trends.back().push_candle(candle);
        if (trends.back().error > max_allowed_error) {
            // finalize current trend and start a new one
            this->push_into_new_trend(candle);
        }
    }
}

void TrendDetector::push_into_new_trend(const Candle& candle) {
    // Logic to push candle into a new trend will go here
    Trend new_trend;
    new_trend.start_ts = candle.t;
    new_trend.start_price = candle.vwap;
    new_trend.push_candle(candle);
    trends.push_back(std::move(new_trend));
}

void Trend::push_candle(const Candle& candle) {
    // Optimized O(1) implementation using running accumulators

    // NOTE: Uncomment the line below if you need to store individual candles for later use
    // this->candles.push_back(candle);  // deque has O(1) push_back without reallocation

    this->end_ts = candle.candle_end_time();
    this->end_price = candle.vwap;

    // Extract values for this candle
    double x = static_cast<double>(candle_count);
    double y = candle.vwap;
    double w = candle.v;

    // Push to zigzag indicators
    this->zigzag_001->push(candle.t, candle.vwap);
    this->zigzag_003->push(candle.t, candle.vwap);

    // Update all 6 accumulators incrementally (O(1) operation)
    acc_w += w;
    acc_wx += w * x;
    acc_wy += w * y;
    acc_wxx += w * x * x;
    acc_wxy += w * x * y;
    acc_wyy += w * y * y;

    candle_count++;

    if (acc_w == 0) {
        slope = 0;
        this->intercept = 0;
        error = 0;
        r_squared = 0;
        return;
    }

    // Calculate slope and intercept using accumulators
    double denominator = acc_w * acc_wxx - acc_wx * acc_wx;

    if (abs(denominator) < 1e-10) {
        slope = 0;
        this->intercept = acc_wy / acc_w;
    } else {
        slope = (acc_w * acc_wxy - acc_wx * acc_wy) / denominator;
        this->intercept = (acc_wy - slope * acc_wx) / acc_w;
    }

    // Calculate error metrics using closed-form formulas (O(1) operation)
    double weighted_mean = acc_wy / acc_w;

    // Sum of weighted squared total deviations: Σwᵢ(yᵢ - ȳ)² = Σwᵢyᵢ² - (Σwᵢyᵢ)²/Σwᵢ
    double sum_weighted_sq_total = acc_wyy - (acc_wy * acc_wy) / acc_w;

    // Sum of weighted squared errors: Σwᵢ(yᵢ - ŷᵢ)² = Σwᵢyᵢ² - slope·Σwᵢxᵢyᵢ - intercept·Σwᵢyᵢ
    double sum_weighted_sq_error = acc_wyy - slope * acc_wxy - intercept * acc_wy;

    if (sum_weighted_sq_error < 0) {
        // Numerical precision issue - clamp to zero
        sum_weighted_sq_error = 0;
    }

    error = sqrt(sum_weighted_sq_error / acc_w);

    if (sum_weighted_sq_total > 0) {
        r_squared = 1.0 - (sum_weighted_sq_error / sum_weighted_sq_total);
    } else {
        r_squared = 1.0;
    }
}

void Trend::push_candle_old(const Candle& candle) {
    cout << "push candle old called" << endl;
    // Recalculate weighted linear regression with new candle
    this->candles.push_back(candle);
    this->end_ts = candle.candle_end_time();
    this->end_price = candle.vwap;
    
    double sum_w = 0, sum_wx = 0, sum_wy = 0, sum_wxx = 0, sum_wxy = 0;

    for (size_t i = 0; i < candles.size(); i++) {
        double x = static_cast<double>(i);
        double y = candles[i].vwap;
        double w = candles[i].v;

        sum_w += w;
        sum_wx += w * x;
        sum_wy += w * y;
        sum_wxx += w * x * x;
        sum_wxy += w * x * y;
    }

    if (sum_w == 0) {
        slope = 0;
        error = 0;
        return;
    }

    // Calculate slope and intercept
    double denominator = sum_w * sum_wxx - sum_wx * sum_wx;
    double intercept = 0;

    if (abs(denominator) < 1e-10) {
        slope = 0;
        intercept = sum_wy / sum_w;
    } else {
        slope = (sum_w * sum_wxy - sum_wx * sum_wy) / denominator;
        intercept = (sum_wy - slope * sum_wx) / sum_w;
    }

    // Calculate weighted error (RMSE) and R²
    double weighted_mean = sum_wy / sum_w;
    double sum_weighted_sq_error = 0;
    double sum_weighted_sq_total = 0;
    double sum_weights = 0;

    for (size_t i = 0; i < candles.size(); i++) {
        double x = static_cast<double>(i);
        double y = candles[i].vwap;
        double predicted = slope * x + intercept;
        double residual = y - predicted;
        double deviation = y - weighted_mean;
        double w = candles[i].v;

        sum_weighted_sq_error += w * residual * residual;
        sum_weighted_sq_total += w * deviation * deviation;
        sum_weights += w;
    }

    if (sum_weights == 0) {
        error = 0;
        r_squared = 0;
    } else {
        error = sqrt(sum_weighted_sq_error / sum_weights);

        if (sum_weighted_sq_total > 0) {
            r_squared = 1.0 - (sum_weighted_sq_error / sum_weighted_sq_total);
        } else {
            r_squared = 1.0;
        }
    }
}
