#pragma once

#include "timestamps.hpp"

#include <string>
#include <vector>

namespace syrinc {
namespace lines {

std::string
correct_line_offset (const std::string &source, const long offset = 0, bool invert_direction = false);

std::vector<syrinc::timestamps::timestamp>
line_timestamps (const std::string &source);

std::string
strip_timestamps (const std::string &source);

}
}