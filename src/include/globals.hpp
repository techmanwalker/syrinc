#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using __dummy_path__ = fs::path;

namespace syrinc {

using filelines = std::vector<std::string>;
using token = std::string; // make the codebase obvious

}