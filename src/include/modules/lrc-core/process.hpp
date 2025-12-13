#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../../globals.hpp"

filelines
process_lyrics (const filelines lyrics, const std::string options = "");

filelines
process_lyrics (const fs::path lyrics, const std::string options);