#include <string>
#include <vector>

std::vector<std::string>
tokenize_lyric_line (const std::string source);

std::string
serialize_lyric_tokens (const std::vector<std::string> token_vector);