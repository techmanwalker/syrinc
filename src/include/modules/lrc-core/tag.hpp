#pragma once

#include <string>
#include <vector>

struct tag {
    std::string name;
    std::string value;
};

std::vector<tag>
read_tags_from_line (const std::string_view source);

tag
slice_at_character (const std::string_view source, char joint = ' ');

std::string
pop_tag (std::string source, std::string key);