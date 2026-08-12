#pragma once

#include <string>

namespace syrinc {
namespace lines {

std::string
correct_line_offset (const std::string source, const long offset = 0, bool invert_direction = false);

}
}