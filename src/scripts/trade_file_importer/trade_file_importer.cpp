#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include "../../libs/core/config/config.hpp"
#include "../../libs/trade/trade.hpp"
#include "../../libs/utils/datetime_utils.hpp"
#include "../../libs/utils/string_utils.hpp"



using namespace std;

void read_binance_trade_file_and_append_to_binary_file(const string& input_file, const string& output_file) {
    // read trade csv file line by line and append to binary file
    // csv file has heade in this format id,price,qty,quote_qty,time,is_buyer_maker
    // ignore the header line
    // caution: numbers may be in scientific notation
    // records must be places in Trade class format
    // output binary file must be created if it does not exist and appended if it exists
    ifstream infile(input_file);
    if (!infile.is_open()) {
        cout << "Error: Could not open input file: " << input_file << endl;
        return;
    }
    ofstream outfile(output_file, ios::out | ios::binary | ios::app);
    if (!outfile.is_open()) {
        cout << "Error: Could not open output file: " << output_file << endl;
        return;
    }
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
        // Write Trade object to binary file
        outfile.write(reinterpret_cast<const char*>(&trade), sizeof(Trade));
    }
    infile.close();
    outfile.close();
}

void multi_binance_trade_daily_file_importer(string symbol, int start_year, int start_month, int start_day, int number_of_days) {
    Config & config = Config::getInstance();
    utils::YearMonthDay ymd(start_year, start_month, start_day);
    for (int i = 0; i < number_of_days; i++) {
        string input_file = config.data_path + "um/trades/" + utils::toUpperCase(symbol) + "-trades-" + ymd.to_string() + ".csv";
        string output_file = config.data_path + "um/trades/" + utils::toLowerCase(symbol) + ".bin";
        cout << "Importing " << input_file << " to " << output_file << endl;
        read_binance_trade_file_and_append_to_binary_file(input_file, output_file);
        ymd = ymd + 1;
    }
}


int main(int argc, char* argv[]) {
    if (argc != 6) {
        cout << "Usage: " << argv[0] << " <symbol> <start_year> <start_month> <start_day> <number_of_days>" << endl;
        return 1;
    }
    string symbol = argv[1];
    int start_year = atoi(argv[2]);
    int start_month = atoi(argv[3]);
    int start_day = atoi(argv[4]);
    int number_of_days = atoi(argv[5]);
    multi_binance_trade_daily_file_importer(symbol, start_year, start_month, start_day, number_of_days);
    return 0;
}