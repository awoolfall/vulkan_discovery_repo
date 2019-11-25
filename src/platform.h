#pragma once

#include <EASTL/string.h>

// converts relative path to an absolute path
eastl::string to_absolute_path(eastl::string pRelativePath);

// reads a string file using an absolute file path
eastl::string read_string_from_file(eastl::string pAbsolutePath);