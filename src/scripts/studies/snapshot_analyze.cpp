#include "../../studies/snapshot_analyze/snapshot_analyze.hpp"
#include "../../libs/core/config/config.hpp"
#include "../../libs/utils/random_utils.hpp"
#include <random>
#include <vector>

int main(int argc, char* argv[]) {
    string symbol;
    size_t ts_seconds;
    double pips_threshold = 30;
    long long lowess_half_neighbor = 900;

    if (argc > 1 && string(argv[1]) == "config") {
        // Read from config file
        Config& config = Config::getInstance();
        symbol = config.get("snapshot_symbol");
        ts_seconds = config.get_timestamp("snapshot_timestamp") / 1000;
        pips_threshold = config.get_double("snapshot_pips_threshold");
        if (config.exist("lowess.half_neighbor")) {
            lowess_half_neighbor = config.get_int("lowess.half_neighbor");
        }

        cout << "Reading from config file:" << endl;
    } else {
        // Random selection
        std::mt19937 rng(42); // fixed seed for reproducibility

        vector<string> symbols = {"btcusdt", "ethusdt", "adausdt", "xrpusdt", "vineusdt", "trumpusdt", "dogeusdt"};

        // valid range: 2025-03-17 to 2025-04-03
        size_t start_ts = utils::get_timestamp("2025-03-17 00:00:00");
        size_t end_ts = utils::get_timestamp("2025-04-03 00:00:00");

        std::uniform_int_distribution<size_t> dist_ts(start_ts / 1000, end_ts / 1000);
        std::uniform_int_distribution<size_t> dist_symbol(0, symbols.size() - 1);

        ts_seconds = dist_ts(rng);
        symbol = symbols[dist_symbol(rng)];

        cout << "Using random selection:" << endl;
    }

    cout << "Creating snapshot analyzer with:" << endl;
    cout << "  Symbol: " << symbol << endl;
    cout << "  Timestamp: " << ts_seconds << "s (" << utils::get_utc_datetime_string(ts_seconds * 1000) << ")" << endl;
    cout << "  Pips threshold: " << pips_threshold << endl;
    cout << "  LOWESS half_neighbor: " << lowess_half_neighbor << endl;
    cout << endl;

    SnapshotAnalyze analyzer(symbol, ts_seconds);
    analyzer.pips_threshold_for_segmentation = pips_threshold;
    analyzer.lowess_half_neighbor = lowess_half_neighbor;
    analyzer.analyze();
    analyzer.print_summary();
    analyzer.export_to_binary();
    analyzer.export_segments();

    return 0;
}
