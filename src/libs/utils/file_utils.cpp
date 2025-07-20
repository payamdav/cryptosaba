#include "file_utils.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>


using namespace std;

namespace utils {

bool is_path_exists(const std::string& path) {
    return filesystem::exists(path);
}

void create_directory(const std::string& path) {
    filesystem::create_directories(path);
}

void remove_directory(const std::string& path) {
    filesystem::remove_all(path);
}

void remove_file(const std::string& path) {
    filesystem::remove(path);
}

std::string get_current_directory() {
    return filesystem::current_path().string();
}

bool is_file_exists(const std::string& path) {
    return filesystem::exists(path) && filesystem::is_regular_file(path);
}

void unzip_file(const std::string& zip_file, const std::string& dest_dir) {
    string dest = dest_dir;
    if (dest_dir.empty()) {
        filesystem::path p(zip_file);
        dest = p.parent_path().string();
    }
    filesystem::create_directories(dest);
    string command = "unzip -o " + zip_file + " -d " + dest;
    if (system(command.c_str()) != 0) {
        throw runtime_error("Error unzipping file: " + zip_file);
    }
}

vector<string> list_files(const string& path) {
    vector<string> files;
    for (const auto& entry : filesystem::directory_iterator(path)) {
        files.push_back(entry.path().string());
    }
    return files;
}

vector<string> list_files(const string& path, const string& ext) {
    vector<string> files;
    for (const auto& entry : filesystem::directory_iterator(path)) {
        if (entry.path().extension() == ext) {
            files.push_back(entry.path().string());
        }
    }
    return files;
}

void touch_file(const string& path) {
    ofstream file(path, ios::app);
    if (!file) {
        throw runtime_error("Error creating file: " + path);
    }
    file.close();
}

size_t get_file_size(const string& path) {
    ifstream file(path, ios::binary | ios::ate);
    if (!file) {
        throw runtime_error("Error opening file: " + path);
    }
    return file.tellg();
}

}

