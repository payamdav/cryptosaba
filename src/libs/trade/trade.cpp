#include "trade.hpp"
#include <fstream>
#include "../core/config/config.hpp"
#include "../utils/file_utils.hpp"
#include "../core/pubsub/pubsub.hpp"
#include <iostream>


using namespace std;

// operator overload for << to print Trade objects
ostream& operator<<(ostream& os, const Trade& trade) {
    os << "Trade(p: " << trade.p 
       << ", v: " << trade.v 
       << ", q: " << trade.q 
       << ", t: " << trade.t 
       << ", is_buyer_maker: " << (trade.is_buyer_maker ? "true" : "false") 
       << ")";
    return os;
}


bool TradeReader::read_trade(size_t index, Trade &trade) {
    if (index >= count) return false;
    trade_data.seekg(index * sizeof(Trade), ios::beg); // Move to the correct position in the file
    trade_data.read(reinterpret_cast<char*>(&trade), sizeof(Trade)); // Read the trade data
    return true; // Return true if the trade was successfully read
}

Trade TradeReader::read_trade(size_t index) {
    Trade trade;
    if (!read_trade(index, trade)) {
        cout << "Error: Unable to read trade at index: " << index << endl;
        // Handle the error as needed, e.g., return a default trade or throw an exception
    }
    return trade; // Return the trade object
}

Trade TradeReader::read_first() {
    if (count == 0) {
        throw runtime_error("No trades available to read.");
    }
    return read_trade(0); // Read the first trade
}

Trade TradeReader::read_last() {
    if (count == 0) {
        throw runtime_error("No trades available to read.");
    }
    return read_trade(count - 1); // Read the last trade
}

size_t TradeReader::search(size_t t) {
    // Binary search for the trade with timestamp t or greater than t
    size_t left = 0;
    size_t right = count;
    size_t mid;
    Trade mid_trade;
    while (left < right) {
        mid = left + (right - left) / 2; // Calculate the middle index
        read_trade(mid, mid_trade); // Read the trade at the middle index
        if (mid_trade.t < t) {
            left = mid + 1; // Search in the right half
        } else {
            right = mid; // Search in the left half
        }
    }
    return left; // Return the index of the first trade with timestamp >= t    
}


TradeReader::TradeReader(string symbol) {

    this->symbol = symbol;
    size_t file_size = utils::get_file_size(Config::getInstance().data_path + "um/trades/" + this->symbol + ".bin");
    this->count = file_size / sizeof(Trade); // Calculate the number of trades in the binary file
    open(); // Open the binary file for reading
}

TradeReader::~TradeReader() {
    close(); // Close the file if it's open
}

void TradeReader::open() {
    trade_data.open(Config::getInstance().data_path + "um/trades/" + this->symbol + ".bin", ios::in | ios::binary); // Open the binary file for reading
    if (!trade_data.is_open()) {
        cout << "Error: Could not open trade data file: " << symbol << endl;
        return; // Return early if the file cannot be opened
    }
}

void TradeReader::close() {
    if (trade_data.is_open()) {
        trade_data.close(); // Close the file if it's open
    } else {
        cout << "Trade data file for symbol: " << symbol << " is not open." << endl;
    }
}


void TradeReader::set_file_cursor(size_t pos) {
    trade_data.seekg(pos * sizeof(Trade), ios::beg); // Move to the specified position
}

void TradeReader::next(Trade &trade) {
    trade_data.read(reinterpret_cast<char*>(&trade), sizeof(Trade)); // Read the next trade
}

void TradeReader::pubsub_trades(size_t t1, size_t t2) {
    size_t start_index = t1 == 0 ? 0 : search(t1); // Find the starting index for the given timestamp
    size_t end_index = t2 == 0 ? count - 1 : search(t2); // Find the ending index for the given timestamp
    // cout << "Publishing trades from index " << start_index << " to " << end_index << endl;
    Trade trade;
    PubSub & pubsub = PubSub::getInstance(); // Get the instance of the PubSub class
    set_file_cursor(start_index); // Set the file cursor to the starting index
    size_t trade_count = end_index - start_index + 1; // Calculate the number of trades to publish
    while (trade_count--) {
        next(trade); // Read the next trade
        pubsub.publish("trade", &trade); // Publish the trade data
    }
    // cout << "Published trades from index " << start_index << " to " << end_index << endl;
    pubsub.publish("trade_finished", nullptr); // Publish a null trade to indicate the end of the stream
}

vector<Trade> TradeReader::read_by_ts_to_vector(size_t t1, size_t t2) {
    vector<Trade> trades;
    size_t start_index = t1 == 0 ? 0 : search(t1); // Find the starting index for the given timestamp
    size_t end_index = t2 == 0 ? count - 1 : search(t2); // Find the ending index for the given timestamp
    Trade trade;
    set_file_cursor(start_index); // Set the file cursor to the starting index
    size_t trade_count = end_index - start_index + 1; // Calculate the number of trades to read
    while (trade_count--) {
        next(trade); // Read the next trade
        trades.push_back(trade); // Add the trade to the vector
    }
    return trades; // Return the vector of trades
}

// utilities related to trades

void write_trades_to_bin_file(string file_path_name, const vector<Trade>& trades) {
    ofstream trade_data(file_path_name, ios::out | ios::binary); // Open the binary file for writing
    if (!trade_data.is_open()) {
        cout << "Error: Could not open file for writing trades: " << file_path_name << endl;
        return; // Return early if the file cannot be opened
    }
    for (const auto& trade : trades) {
        // write fields one by one in binary format
        trade_data.write(reinterpret_cast<const char*>(&trade.p), sizeof(trade.p));
        trade_data.write(reinterpret_cast<const char*>(&trade.v), sizeof(trade.v));
        trade_data.write(reinterpret_cast<const char*>(&trade.q), sizeof(trade.q));
        trade_data.write(reinterpret_cast<const char*>(&trade.t), sizeof(trade.t));
    }
    trade_data.close(); // Close the file after writing
}

void write_trades_to_bin_file_price_ts(string file_path_name, const vector<Trade>& trades) {
    ofstream trade_data(file_path_name, ios::out | ios::binary); // Open the binary file for writing
    if (!trade_data.is_open()) {
        cout << "Error: Could not open file for writing trades: " << file_path_name << endl;
        return; // Return early if the file cannot be opened
    }
    for (const auto& trade : trades) {
        // write only price and timestamp fields in binary format
        trade_data.write(reinterpret_cast<const char*>(&trade.t), sizeof(trade.t));
        trade_data.write(reinterpret_cast<const char*>(&trade.p), sizeof(trade.p));
    }
    trade_data.close(); // Close the file after writing
}