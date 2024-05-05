/**
 * @file ini_handler.cpp
 * @brief Implementation file for library.
 */

#include "ini_handler.h"
#include <fstream>
#include <iostream>
#include <unordered_map>

// Global variable to hold the key-value pairs from the INI file
std::unordered_map<std::string, std::string> ini_data;
std::string ini_file_path;

unsigned short load_resource(const std::string& path) {

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << path << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        // TODO implement ini parsing logic here
        std::cout << line << std::endl;
    }

    // Check for errors while reading the file
    if (file.bad()) {
        std::cerr << "Error: Read error occurred while reading file " << path << std::endl;
        return 1;
    }

    file.close();
    return 0;
}

unsigned short get_value(const std::string& key, std::string& value) {
    // Check if a resource file has been loaded
    if (ini_data.empty()) {
        std::cerr << "Error: No resource file has been loaded yet" << std::endl;
        return 4; // Resource file not loaded
    }

    // Find the key in the map
    auto it = ini_data.find(key);
    if (it == ini_data.end()) {
        std::cerr << "Error: Key '" << key << "' not found in the resource file" << std::endl;
        return 3; // Key not found
    }

    // Retrieve the value
    value = it->second;

    // Return success
    return 0;
}

unsigned short set_value(const std::string& key, const std::string& value) {
    // Check if a resource file has been loaded
    if (ini_data.empty()) {
        std::cerr << "Error: No resource file has been loaded yet" << std::endl;
        return 4; // Resource file not loaded
    }

    // Update the volatile memory
    ini_data[key] = value;

    // Update the INI file on the filesystem
    std::ofstream file(ini_file_path);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << ini_file_path << " for writing" << std::endl;
        return 255; // Generic error
    }

    for (const auto& entry : ini_data) {
        file << entry.first << "=" << entry.second << std::endl;
    }

    // Check for errors while writing to the file
    if (file.fail()) {
        std::cerr << "Error: Write error occurred while writing to file " << ini_file_path << std::endl;
        return 255; // Generic error
    }

    // Close the file
    file.close();

    // Return success
    return 0;
}