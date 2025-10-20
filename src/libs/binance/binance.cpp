#include "binance.hpp"
#include <iostream>
#include <curl/curl.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <sstream>

#include "../utils/datetime_utils.hpp"
#include "../utils/string_utils.hpp"
#include "../utils/file_utils.hpp"
#include "../trade/trade.hpp"
#include "../core/pubsub/pubsub.hpp"


string binance_trade_futures_url(const string& symbol, int year, int month, int day) {
    if (day > 0) {
        utils::YearMonthDay ymd(year, month, day);
        return format("https://data.binance.vision/data/futures/um/daily/trades/{}/{}-trades-{}.zip", utils::toUpperCase(symbol), utils::toUpperCase(symbol), ymd.to_string());
    }
    else {
        utils::YearMonth ym(year, month);
        return format("https://data.binance.vision/data/futures/um/monthly/trades/{}/{}-trades-{}.zip", utils::toUpperCase(symbol), utils::toUpperCase(symbol), ym.to_string());
    }
}


string prepare_destination_directory() {
    // Get home directory
    const char* home_dir = getenv("HOME");
    if (!home_dir) {
        struct passwd* pw = getpwuid(getuid());
        home_dir = pw->pw_dir;
    }

    // Create binance_data directory if it doesn't exist
    string data_dir = string(home_dir) + "/binance_data";
    if (!utils::is_path_exists(data_dir)) {
        utils::create_directory(data_dir);
    }
    return data_dir;
}

// Callback function for libcurl to write data to file
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

string download_binance_trade_data(const string& symbol, int year, int month, int day) {
    // Prepare destination directory
    string data_dir = prepare_destination_directory();

    // Get the URL and extract filename from it
    string url = binance_trade_futures_url(symbol, year, month, day);

    // Extract filename from URL (last part after the final '/')
    size_t last_slash = url.find_last_of('/');
    string filename = url.substr(last_slash + 1);

    string output_path = data_dir + "/" + filename;

    std::cout << "Downloading: " << url << std::endl;
    std::cout << "To: " << output_path << std::endl;

    // Initialize curl
    CURL *curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return"";
    }

    // Open file for writing
    FILE *fp = fopen(output_path.c_str(), "wb");
    if (!fp) {
        std::cerr << "Failed to open file for writing: " << output_path << std::endl;
        curl_easy_cleanup(curl);
        return"";
    }

    // Set curl options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    // Perform the download
    CURLcode res = curl_easy_perform(curl);

    // Cleanup
    fclose(fp);

    if (res != CURLE_OK) {
        std::cerr << "Download failed: " << curl_easy_strerror(res) << std::endl;
        utils::remove_file(output_path);
    } else {
        std::cout << "Download completed successfully!" << std::endl;

        // Unzip the file
        std::cout << "Unzipping file..." << std::endl;
        utils::unzip_file(output_path, data_dir);
        std::cout << "Unzip completed!" << std::endl;

        // Remove the zip file
        std::cout << "Removing zip file..." << std::endl;
        utils::remove_file(output_path);
        std::cout << "Cleanup completed!" << std::endl;
    }

    curl_easy_cleanup(curl);
    // return the path to the unzipped data file - remove .zip extension and add .csv
    string unzipped_filename = filename.substr(0, filename.find_last_of('.')) + ".csv";
    return data_dir + "/" + unzipped_filename;
}


string publish_trades_from_csv_file(const string& symbol, int year, int month, int day) {
    // Download the trade data file
    string csv_file_path = download_binance_trade_data(symbol, year, month, day);
    if (csv_file_path.empty()) {
        std::cerr << "Failed to download trade data." << std::endl;
        return "";
    }

    PubSub& pubsub = PubSub::getInstance();

    // Read the CSV file and publish trades
    std::ifstream infile(csv_file_path);
    if (!infile.is_open()) {
        std::cerr << "Failed to open CSV file: " << csv_file_path << std::endl;
        return "";
    }

    double epsilon = 1e-6;
    size_t last_ts = 0;

    string line;
    // skip header line
    getline(infile, line);
    size_t line_number = 1;
    while (getline(infile, line)) {
        line_number++;
        stringstream ss(line);
        string item;
        vector<string> tokens;
        while (getline(ss, item, ',')) {
            tokens.push_back(item);
        }
        if (tokens.size() != 6) {
            cerr << "Error: Invalid line format at line " << line_number << ": " << line << endl;
            continue;
        }
        // Create Trade object from tokens
        Trade trade;
        try {
            trade.p = stod(tokens[1]);
            trade.v = stod(tokens[2]);
            trade.q = stod(tokens[3]);
            trade.t= stoll(tokens[4]);
            trade.is_buyer_maker = (tokens[5] == "true");
        } catch (const exception& e) {
            cerr << "Error: Failed to parse line " << line_number << ": " << e.what() << endl;
            continue;
        }

        // check integrity of trade data
        if (trade.t < last_ts) {
            cerr << csv_file_path << " >> Error: Timestamps not in ascending order at line " << line_number << ": " << trade.t << " < " << last_ts << endl;
            continue;
        }
        last_ts = trade.t;

        if (trade.q <= epsilon) {
            cerr << csv_file_path << " >> Error: Invalid quantity at line " << line_number << ": " << trade.q << endl;
            continue;
        }
        if (trade.v <= epsilon) {
            cerr << csv_file_path << " >> Error: Invalid volume at line " << line_number << ": " << trade.v << endl;
            continue;
        }
        if (abs((trade.q / trade.v) - trade.p) > epsilon) {
            // try to correct q befor reporting error
            if (trade.q < 10) {
                trade.q = trade.p * trade.v;
                if (abs((trade.q / trade.v) - trade.p) <= epsilon) {
                    // correction successful
                    // nothing to do
                } else {
                    // correction failed
                    cerr << csv_file_path << " >> Error: Failed to correct quantity at line " << line_number << ": p=" << trade.p << ", q=" << trade.q << ", v=" << trade.v << " - correction failed!" << endl;
                    continue;
            }
            } else {
                // q is not small enough to be considered as broken length scientific notation
                // report error
                cerr << csv_file_path << " >> Error: Inconsistent price, quantity, and volume at line " << line_number << ": p=" << trade.p << ", q=" << trade.q << ", v=" << trade.v << endl;
                continue;
            }
        }

        // Publish the trade
        pubsub.publish("trade", &trade);

    }
    infile.close();

    return csv_file_path;
}

