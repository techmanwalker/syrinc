#pragma once

#include <filesystem>

#include "globals.hpp"

namespace syrinc {
namespace process {

filelines
process_lyrics (const filelines lyrics, options o);

filelines
process_lyrics (const fs::path lyrics, options o);

}
}