#pragma once

#include <string>
#include <vector>

std::vector<std::string_view>
tokenize_line (const std::string_view source, bool treat_as_lyrics_line = false);

std::string
serialize_tokens (const std::vector<std::string_view> token_vector, std::string_view joint = " ", bool treat_as_lyrics_line = false);

std::string
serialize_tokens (const std::vector<std::string>& token_vector, std::string_view joint = " ", bool treat_as_lyrics_line = false);

std::string
trim_string (const std::string source);