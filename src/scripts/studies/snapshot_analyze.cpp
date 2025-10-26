#include "../../studies/snapshot_analyze/snapshot_analyze.hpp"
#include "../../libs/utils/random_utils.hpp"
#include <random>
#include <vector>

int main() {
    std::mt19937 rng(42); // fixed seed for reproducibility

    vector<string> symbols = {"btcusdt", "ethusdt", "adausdt", "xrpusdt", "vineusdt", "trumpusdt", "dogeusdt"};

    // valid range: 2025-03-17 to 2025-04-03
    size_t start_ts = utils::get_timestamp("2025-03-17 00:00:00");
    size_t end_ts = utils::get_timestamp("2025-04-03 00:00:00");

    std::uniform_int_distribution<size_t> dist_ts(start_ts / 1000, end_ts / 1000);
    std::uniform_int_distribution<size_t> dist_symbol(0, symbols.size() - 1);

    size_t random_ts_seconds = dist_ts(rng);
    string random_symbol = symbols[dist_symbol(rng)];

    cout << "Creating snapshot analyzer with:" << endl;
    cout << "  Symbol: " << random_symbol << endl;
    cout << "  Timestamp: " << random_ts_seconds << "s (" << utils::get_utc_datetime_string(random_ts_seconds * 1000) << ")" << endl;
    cout << endl;

    SnapshotAnalyze analyzer(random_symbol, random_ts_seconds);
    analyzer.analyze();
    analyzer.print_summary();
    analyzer.export_to_binary();
    analyzer.export_segments();

    return 0;
}
