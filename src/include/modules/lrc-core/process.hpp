#pragma once

#include <filesystem>
#include <string>

#include "globals.hpp"

namespace syrinc {
namespace process {

filelines
process_lyrics (const filelines lyrics, const std::string options = "");

filelines
process_lyrics (const fs::path lyrics, const std::string options);

}
}