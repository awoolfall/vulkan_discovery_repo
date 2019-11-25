#pragma once

#include <string>

// converts relative path to an absolute path
std::string to_absolute_path(std::string pRelativePath);

// reads a string file using an absolute file path
std::string read_string_from_file(std::string pAbsolutePath);