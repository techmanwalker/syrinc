#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using filelines = std::vector<std::string>;

filelines
process_lyrics (const filelines lyrics, const std::string options = "");

filelines
process_lyrics (const fs::path lyrics, const std::string options);