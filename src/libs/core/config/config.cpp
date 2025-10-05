#include "config.hpp"
#include <iostream>
#include <fstream>
#include "../../utils/string_utils.hpp"
#include "../../utils/datetime_utils.hpp"
#include <cstdlib>


using namespace std;


Config& Config::getInstance() {
    static Config instance; // Guaranteed to be destroyed.
    return instance;        // Instantiated on first use.
}


Config::Config() {
    home_path = getenv("HOME");
    if (home_path.empty()) {
        cerr << "Error: HOME environment variable is not set." << endl;
        exit(EXIT_FAILURE);
    }
    load();
}

void Config::load() {
    string config_file_path = home_path + "/" + CONFIG_FILE;
    ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        cerr << "Error: Could not open config file at " << config_file_path << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    while (getline(config_file, line)) {
        if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        auto pos = line.find('=');
        if (pos == string::npos) continue; // Skip lines without '='
        string key = utils::trim(line.substr(0, pos));
        string value = utils::trim(line.substr(pos + 1));
        kv[key] = value;
    }
    config_file.close();
    // Set paths based on config values
    if (exist("data_path")) {
        data_path = home_path + "/" + kv["data_path"];
    } else {
        data_path = home_path + "/data/"; // Default data path
    }
    if (exist("files_path")) {
        files_path = home_path + "/" + kv["files_path"];
    } else {
        files_path = home_path + "data/files/"; // Default files path
    }
        
}

bool Config::exist(const string& key) {
    return kv.find(key) != kv.end();
}

string Config::get(const string& key) {
    if (exist(key)) {
        return kv[key];
    } else {
        cerr << "Warning: Key '" << key << "' does not exist in config." << endl;
        return "";
    }
}

long long Config::get_int(const string& key) {
    if (exist(key)) {
        return stoll(kv[key]);
    } else {
        cerr << "Warning: Key '" << key << "' does not exist in config." << endl;
        return 0;
    }
}

double Config::get_double(const string& key) {
    if (exist(key)) {
        return stod(kv[key]);
    } else {
        cerr << "Warning: Key '" << key << "' does not exist in config." << endl;
        return 0.0;
    }
}

std::size_t Config::get_size_t(const string& key) {
    if (exist(key)) {
        return static_cast<std::size_t>(stoull(kv[key]));
    } else {
        cerr << "Warning: Key '" << key << "' does not exist in config." << endl;
        return 0;
    }
}

long long Config::get_timestamp(const string& key) {
    if (exist(key)) {
        return utils::get_timestamp(kv[key]);
    } else {
        cerr << "Warning: Key '" << key << "' does not exist in config." << endl;
        return 0;
    }
}

std::string Config::get_path(const string& key) {
    if (exist(key)) {
        return home_path + "/" + kv[key];
    } else {
        cerr << "Warning: Key '" << key << "' does not exist in config." << endl;
        return "";
    }
}

std::vector<std::string> Config::get_csv_strings(const string& key) {
    if (exist(key)) {
        return utils::split_to_vector(kv[key], ",");
    } else {
        cerr << "Warning: Key '" << key << "' does not exist in config." << endl;
        return {};
    }
}

