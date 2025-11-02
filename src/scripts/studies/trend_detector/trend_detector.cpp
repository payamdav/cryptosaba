#include "trend_detector.hpp"
#include "../../../libs/core/pubsub/pubsub.hpp"
#include <cmath>




void TrendDetector::feed_from_candles(string symbol, size_t start_ts, size_t end_ts) {
    // Instantiate candle reader
    CandleReader reader(symbol, 1);

    // Subscribe to published candles
    PubSub& pubsub = PubSub::getInstance();
    pubsub.subscribe("candle", [this](const void* data) {
        const Candle* candle = static_cast<const Candle*>(data);
        this->push_candle(*candle);
    });

    // Publish candles from reader
    reader.publish(start_ts, end_ts);
}

void TrendDetector::push_candle(const Candle& candle) {
    if (trends.empty()) {
        this->push_into_new_trend(candle);
    } else {
        // Logic to determine if candle continues current trend or starts new trend
        current_trend.push_candle(candle);
        if (current_trend.error > max_allowed_error) {
            // finalize current trend and start a new one
            this->push_into_new_trend(candle);
        } else {
            // continue current trend
            trends.back() = current_trend;
        }
    }
}

void TrendDetector::push_into_new_trend(const Candle& candle) {
    // Logic to push candle into a new trend will go here
    current_trend = Trend();
    current_trend.start_ts = candle.t;
    current_trend.start_price = candle.vwap;
    current_trend.push_candle(candle);
    trends.push_back(current_trend);
}

void Trend::push_candle(const Candle& candle) {
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
