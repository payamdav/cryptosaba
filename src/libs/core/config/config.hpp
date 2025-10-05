#pragma once
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <string>

#define CONFIG_FILE "data/config.conf" // relative to home path

using namespace std;





class Config {
    public:
        static Config& getInstance();
    private:
        Config();
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;
        void load();
    public:
        std::unordered_map<std::string, std::string> kv;

        std::string home_path;
        std::string data_path;
        std::string files_path;
        bool exist(const string& key);
        string get(const string& key);
        long long get_int(const string& key);
        double get_double(const string& key);
        std::size_t get_size_t(const string& key);
        long long get_timestamp(const string& key);
        std::string get_path(const string& key);
        std::vector<std::string> get_csv_strings(const string& key);
        bool get_bool(const string& key);

};
