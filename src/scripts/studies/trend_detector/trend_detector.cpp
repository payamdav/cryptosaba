#include "trend_detector.hpp"
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/core/pubsub/pubsub.hpp"
#include <cmath>

// Initialize static member
SymbolInfo* Trend::symbol_info = nullptr;


SymbolInfo::SymbolInfo() :
    candles_1s(1, 7 * 24 * 60 * 60),
    candles_1m(60, 7 * 24 * 60),
    candles_1h(3600, 7 * 24),
    avg_candle_size_1s(7 * 24 * 60 * 60),
    avg_candle_size_1m(7 * 24 * 60),
    avg_candle_size_1h(7 * 24),
    avg_vol_1s(7 * 24 * 60 * 60) {
    // Initialize any other members if necessary
}

void SymbolInfo::push_candle_1s(const Candle& candle) {
    this->candles_1s.push(candle);
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
        if (candles_1s.full() &&
            candles_1m.full() &&
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
    Config& config = Config::getInstance();
    double delta1 = config.get_double("trend_detector_zigzag_delta_1");
    double delta2 = config.get_double("trend_detector_zigzag_delta_2");
    // Allocate zigzag instances with specified deltas using unique_ptr
    zigzag_1 = make_unique<ZigZag>(delta1, 1000);
    zigzag_2 = make_unique<ZigZag>(delta2, 1000);
}

TrendDetector::TrendDetector() {
    // Initialize static symbol_info pointer if not already set
    if (!Trend::symbol_info) {
        Trend::symbol_info = &SymbolInfo::getInstance();
    }
}

void TrendDetector::push_candle(const Candle& candle) {
    if (trends.empty()) {
        this->push_into_new_trend(candle);
    } else {
        // Logic to determine if candle continues current trend or starts new trend
        // Using tail-based error for better responsiveness to recent changes
        trends.back().push_candle(candle, trend_tail_count_for_error);
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
    new_trend.push_candle(candle, trend_tail_count_for_error);
    trends.push_back(std::move(new_trend));
}

void Trend::push_candle_full(const Candle& candle) {
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
    this->zigzag_1->push(candle.t, candle.vwap);
    this->zigzag_2->push(candle.t, candle.vwap);

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

void Trend::push_candle(const Candle& candle, size_t trend_tail_count_for_error) {
    this->end_ts = candle.candle_end_time();
    this->end_price = candle.vwap;

    // Extract values for this candle
    double x = static_cast<double>(candle_count);
    double y = candle.vwap;
    double w = candle.v;

    // Push to zigzag indicators
    this->zigzag_1->push(candle.t, candle.vwap);
    this->zigzag_2->push(candle.t, candle.vwap);

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

    // Calculate error on the tail using SymbolInfo.candles_1s
    double sum_weighted_sq_error = 0;
    double sum_weights = 0;

    // Determine how many candles to use for error calculation
    size_t error_calculation_count = std::min(trend_tail_count_for_error, candle_count);

    // Start index in the trend (for x coordinate)
    size_t start_index = candle_count - error_calculation_count;

    // Access the tail of SymbolInfo.candles_1s
    if (symbol_info && symbol_info->candles_1s.size() > 0) {
        size_t candles_available = symbol_info->candles_1s.size();
        size_t actual_count = std::min(error_calculation_count, candles_available);

        for (size_t i = 0; i < actual_count; ++i) {
            // Access from the end of candles_1s: last candle is at index (size - 1)
            size_t candle_index = candles_available - actual_count + i;
            const Candle& tail_candle = symbol_info->candles_1s[candle_index];

            double current_x = start_index + i;
            double current_y = tail_candle.vwap;
            double current_w = tail_candle.v;

            double predicted_y = slope * current_x + intercept;
            double residual = current_y - predicted_y;
            sum_weighted_sq_error += current_w * residual * residual;
            sum_weights += current_w;
        }
    }

    if (sum_weights > 0) {
        error = sqrt(sum_weighted_sq_error / sum_weights);
    } else {
        error = 0;
    }
}

// Removed push_candle_old - no longer needed after optimization
