#include "snapshot_analyze.hpp"
#include "../../libs/core/config/config.hpp"
#include <ranges>
#include <numeric>
#include <algorithm>
#include <fstream>

SnapshotAnalyze::SnapshotAnalyze(const string& symbol, size_t ts_seconds)
    : symbol(symbol), ts_seconds(ts_seconds), ts_ms(ts_seconds * 1000),
      l7d(1), l1d(1), n1d(1), l1dn1d(1),
      current_vwap(0), avg_candle_size_1w(0), avg_candle_size_to_pips(0), avg_volume_1w(0),
      pips_threshold_for_segmentation(30), max_error_for_segments(0) {
}

void SnapshotAnalyze::load_candles() {
    size_t l7d_start = ts_ms - ONE_WEEK_MS + ONE_SECOND_MS;
    size_t l7d_end = ts_ms;

    size_t l1d_start = ts_ms - ONE_DAY_MS + ONE_SECOND_MS;
    size_t l1d_end = ts_ms;

    size_t n1d_start = ts_ms + ONE_SECOND_MS;
    size_t n1d_end = ts_ms + ONE_DAY_MS;

    size_t l1dn1d_start = ts_ms - ONE_DAY_MS + ONE_SECOND_MS;
    size_t l1dn1d_end = ts_ms + ONE_DAY_MS;

    cout << "Loading candles for " << symbol << " at " << utils::get_utc_datetime_string(ts_ms) << endl;

    l7d.read_from_binary_file(symbol, l7d_start, l7d_end);
    cout << "Loaded l7d: " << l7d.size() << " candles" << endl;

    l1d.read_from_binary_file(symbol, l1d_start, l1d_end);
    cout << "Loaded l1d: " << l1d.size() << " candles" << endl;

    n1d.read_from_binary_file(symbol, n1d_start, n1d_end);
    cout << "Loaded n1d: " << n1d.size() << " candles" << endl;

    l1dn1d.read_from_binary_file(symbol, l1dn1d_start, l1dn1d_end);
    cout << "Loaded l1dn1d: " << l1dn1d.size() << " candles" << endl;
}

void SnapshotAnalyze::set_current_vwap() {
    if (l1d.empty()) {
        current_vwap = 0;
        return;
    }
    current_vwap = l1d.back().vwap;
    cout << "Current vwap: " << current_vwap << endl;
}

void SnapshotAnalyze::calculate_avg_candle_size() {
    if (l7d.empty() || current_vwap == 0) {
        avg_candle_size_1w = 0;
        avg_candle_size_to_pips = 0;
        return;
    }

    auto candle_sizes = l7d | std::views::transform([](const Candle& c) { return c.h - c.l; });
    avg_candle_size_1w = std::reduce(candle_sizes.begin(), candle_sizes.end(), 0.0) / l7d.size();
    avg_candle_size_to_pips = (avg_candle_size_1w / current_vwap) * 10000;
    cout << "Average candle size (1w): " << avg_candle_size_1w << " (" << avg_candle_size_to_pips << " pips)" << endl;
}

void SnapshotAnalyze::calculate_avg_volume() {
    if (l7d.empty()) {
        avg_volume_1w = 0;
        return;
    }

    auto volumes = l7d | std::views::transform([](const Candle& c) { return c.v; });
    avg_volume_1w = std::reduce(volumes.begin(), volumes.end(), 0.0) / l7d.size();
    cout << "Average volume (1w): " << avg_volume_1w << endl;
}

void SnapshotAnalyze::normalize_volumes() {
    volume_normalized.clear();
    l1dn1d_volume_normalized.clear();
    if (avg_volume_1w == 0) return;

    auto normalized_l1d = l1d | std::views::transform([this](const Candle& c) {
        return c.v / avg_volume_1w;
    });
    volume_normalized.assign(normalized_l1d.begin(), normalized_l1d.end());
    cout << "Normalized " << volume_normalized.size() << " volume values (l1d)" << endl;

    auto normalized_l1dn1d = l1dn1d | std::views::transform([this](const Candle& c) {
        return c.v / avg_volume_1w;
    });
    l1dn1d_volume_normalized.assign(normalized_l1dn1d.begin(), normalized_l1dn1d.end());
    cout << "Normalized " << l1dn1d_volume_normalized.size() << " volume values (l1dn1d)" << endl;
}

void SnapshotAnalyze::offset_and_scale_prices() {
    prices_offsetted_scaled.clear();
    l1dn1d_prices_offsetted_scaled.clear();
    if (avg_candle_size_1w == 0) return;

    auto scaled_l1d = l1d | std::views::transform([this](const Candle& c) {
        return (c.vwap - current_vwap) / avg_candle_size_1w;
    });
    prices_offsetted_scaled.assign(scaled_l1d.begin(), scaled_l1d.end());
    cout << "Offset and scaled " << prices_offsetted_scaled.size() << " prices (l1d)" << endl;

    auto scaled_l1dn1d = l1dn1d | std::views::transform([this](const Candle& c) {
        return (c.vwap - current_vwap) / avg_candle_size_1w;
    });
    l1dn1d_prices_offsetted_scaled.assign(scaled_l1dn1d.begin(), scaled_l1dn1d.end());
    cout << "Offset and scaled " << l1dn1d_prices_offsetted_scaled.size() << " prices (l1dn1d)" << endl;
}

void SnapshotAnalyze::analyze() {
    load_candles();
    set_current_vwap();
    calculate_avg_candle_size();
    calculate_avg_volume();
    normalize_volumes();
    offset_and_scale_prices();
    calculate_segments();
}

void SnapshotAnalyze::calculate_segments() {
    if (prices_offsetted_scaled.empty() || volume_normalized.empty()) return;
    if (current_vwap == 0 || avg_candle_size_1w == 0) return;

    // Calculate max_error from pips threshold
    max_error_for_segments = (pips_threshold_for_segmentation * current_vwap) / (10000.0 * avg_candle_size_1w);
    cout << "Max error for segments (from " << pips_threshold_for_segmentation << " pips): " << max_error_for_segments << endl;

    SegmentedWeightedLinearRegression reg(prices_offsetted_scaled, volume_normalized, max_error_for_segments);
    segments = reg.segments;

    cout << "Segments (volume-weighted): " << segments.size() << endl;
}

void SnapshotAnalyze::print_summary() {
    cout << "\n=== Snapshot Analyze Summary ===" << endl;
    cout << "Symbol: " << symbol << endl;
    cout << "Timestamp: " << utils::get_utc_datetime_string(ts_ms) << " (" << ts_seconds << "s)" << endl;
    cout << "\nCandle counts:" << endl;
    cout << "  l7d: " << l7d.size() << endl;
    cout << "  l1d: " << l1d.size() << endl;
    cout << "  n1d: " << n1d.size() << endl;
    cout << "  l1dn1d: " << l1dn1d.size() << endl;
    cout << "\nMetrics:" << endl;
    cout << "  Current vwap: " << current_vwap << endl;
    cout << "  Avg candle size (1w): " << avg_candle_size_1w << " (" << avg_candle_size_to_pips << " pips)" << endl;
    cout << "  Avg volume (1w): " << avg_volume_1w << endl;
    cout << "  Pips threshold for segmentation: " << pips_threshold_for_segmentation << endl;
    cout << "  Max error for segments: " << max_error_for_segments << endl;

    cout << "\n=== Segments Volume-Weighted (" << segments.size() << ") ===" << endl;
    for (size_t i = 0; i < segments.size(); i++) {
        const auto& seg = segments[i];
        cout << "  [" << i << "] idx:" << seg.start_idx << "->" << seg.end_idx
             << " slope:" << seg.slope << " intercept:" << seg.intercept
             << " error:" << seg.error << endl;
    }
}

void SnapshotAnalyze::export_to_binary() {
    Config& config = Config::getInstance();
    string base_path = config.files_path;

    // Export l1dn1d candles
    string candles_file = base_path + "snapshot_l1dn1d_candles.bin";
    l1dn1d.write_to_binary_file(candles_file, "full");
    cout << "Exported l1dn1d candles to: " << candles_file << endl;

    // Export l1dn1d normalized volumes
    string volumes_file = base_path + "snapshot_l1dn1d_volumes_normalized.bin";
    ofstream vol_stream(volumes_file, ios::binary);
    vol_stream.write(reinterpret_cast<const char*>(l1dn1d_volume_normalized.data()),
                     l1dn1d_volume_normalized.size() * sizeof(double));
    vol_stream.close();
    cout << "Exported l1dn1d normalized volumes to: " << volumes_file << endl;

    // Export l1dn1d offsetted/scaled prices
    string prices_file = base_path + "snapshot_l1dn1d_prices_scaled.bin";
    ofstream prices_stream(prices_file, ios::binary);
    prices_stream.write(reinterpret_cast<const char*>(l1dn1d_prices_offsetted_scaled.data()),
                        l1dn1d_prices_offsetted_scaled.size() * sizeof(double));
    prices_stream.close();
    cout << "Exported l1dn1d offsetted/scaled prices to: " << prices_file << endl;

    // Export metadata (CSV format)
    string metadata_file = base_path + "snapshot_metadata.csv";
    ofstream meta_stream(metadata_file);
    meta_stream << "symbol,ts_ms,ts_datetime,current_vwap,avg_candle_size_1w,avg_candle_size_to_pips,avg_volume_1w,pips_threshold_for_segmentation,max_error_for_segments\n";
    meta_stream << symbol << ","
                << ts_ms << ","
                << utils::get_utc_datetime_string(ts_ms) << ","
                << current_vwap << ","
                << avg_candle_size_1w << ","
                << avg_candle_size_to_pips << ","
                << avg_volume_1w << ","
                << pips_threshold_for_segmentation << ","
                << max_error_for_segments << "\n";
    meta_stream.close();
    cout << "Exported metadata to: " << metadata_file << endl;
    cout << "\nAll files exported successfully!" << endl;
}

void SnapshotAnalyze::export_segments() {
    Config& config = Config::getInstance();
    string base_path = config.files_path;

    string seg_file = base_path + "snapshot_segments.bin";
    ofstream seg_stream(seg_file, ios::binary);
    size_t count = segments.size();
    seg_stream.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
    for (const auto& seg : segments) {
        seg_stream.write(reinterpret_cast<const char*>(&seg.start_idx), sizeof(size_t));
        seg_stream.write(reinterpret_cast<const char*>(&seg.end_idx), sizeof(size_t));
        seg_stream.write(reinterpret_cast<const char*>(&seg.slope), sizeof(double));
        seg_stream.write(reinterpret_cast<const char*>(&seg.intercept), sizeof(double));
        seg_stream.write(reinterpret_cast<const char*>(&seg.error), sizeof(double));
    }
    seg_stream.close();
    cout << "Exported segments (weighted) to: " << seg_file << endl;
}
