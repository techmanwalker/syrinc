#pragma once

#include <string>
#include <vector>
#include "globals.hpp"

std::vector<token>
tokenize_line (const std::string source, bool treat_as_lyrics_line = false);

std::string
serialize_tokens (const std::vector<token> token_vector, std::string joint = " ", bool treat_as_lyrics_line = false);

std::string
trim_string (const std::string source);