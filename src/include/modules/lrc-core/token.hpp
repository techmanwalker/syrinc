#pragma once

#include <string>
#include <vector>

std::vector<std::string>
tokenize_line (const std::string source, bool treat_as_lyrics_line = false);

std::string
serialize_tokens (const std::vector<std::string> token_vector, std::string joint = " ", bool treat_as_lyrics_line = false);

std::vector<std::pair<std::string, std::string>>
read_tags_from_line (const std::string source);

std::string
trim_string (const std::string source);

std::pair<std::string, std::string>
slice_at_character (const std::string source, char joint = ' ');

std::string
pop_tag (std::string source, std::string key);