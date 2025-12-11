#pragma once

#include <string>

std::string
correct_line_offset (const std::string source, const long offset = 0, bool invert_direction = false);

std::string
apply_offset_to_timestamp (const std::string source, const long offset = 0, bool invert_direction = false);