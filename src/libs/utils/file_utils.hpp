#pragma once
#include <string>
#include <vector>

namespace utils {
    bool is_path_exists(const std::string& path);
    bool is_file_exists(const std::string& path);
    void create_directory(const std::string& path);
    void remove_directory(const std::string& path);
    void remove_file(const std::string& path);
    std::string get_current_directory();
    void unzip_file(const std::string& zip_file, const std::string& dest_dir="");
    std::vector<std::string> list_files(const std::string& path);
    std::vector<std::string> list_files(const std::string& path, const std::string& ext);
    void touch_file(const std::string& path);
    size_t get_file_size(const std::string& path);
}
