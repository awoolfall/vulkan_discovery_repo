#pragma once

#include <string>
#include <vector>

// converts relative path to an absolute path
std::string to_absolute_path(std::string pRelativePath);

// reads a string file using an absolute file path
std::string read_string_from_file(std::string pAbsolutePath);

// reads binary data from a given absolute file path
std::vector<char> read_data_from_binary_file(std::string abs_path);