#pragma once

#include "globals.hpp"

std::vector<tag>
read_tags_from_line (const std::string source);

tag
slice_at_character (const std::string source, char joint = ' ');

std::string
pop_tag (std::string source, std::string key);