/**
* @file token.cpp
* @brief Tools to divide a lyric line in a series of tokens and
* serialize afterwards.
* @par tokenize_lyric_line ("[00:00:00] Beginning of a song");
* @par serialize_lyric_tokens ({"[", "00:12.34", "Another", "part"});
* 
*/

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "token.hpp"

/**
* @brief Split a line in multiple "tokens" in order to be able to
* do the timestamp correction
*
* This function will:
* 1. Split the line by its spaces
* 2. Join timestamps with their aperture and closure symbols if they have
*    to distinguish them from other kinds of data [ ] < >
* 3. Return the tokenized lyric line
*
* @param source the string to be converted to space-sparated tokens.
*/
std::vector<std::string_view>
tokenize_line(std::string_view source, bool treat_as_lyrics_line)
{
    std::vector<std::string_view> tokens;

    size_t token_start = 0;
    bool in_token = false;

    auto flush = [&](size_t end) {
        if (in_token && end > token_start)
            tokens.emplace_back(source.data() + token_start, end - token_start);
        in_token = false;
    };

    for (size_t i = 0; i < source.size(); ++i) {
        char c = source[i];

        if (c == ' ') {
            flush(i);
            continue;
        }

        if (treat_as_lyrics_line &&
            (c == '[' || c == ']' || c == '<' || c == '>'))
        {
            flush(i);
            tokens.emplace_back(source.data() + i, 1); // ← CORRECTO
            continue;
        }

        if (!in_token) {
            token_start = i;
            in_token = true;
        }
    }

    flush(source.size());
    return tokens;
}


/**
* @brief Convert a token list to a single string
*
* Just concatenate a token vector back to a string, but ensuring that 
* timestamp tags stay together like [00:00.00] and don't become sparse
* like [ 00:00:00 ] or < 00:00:00 >
*
* @param source the token vector to be converted back to string
*
*/
std::string
serialize_tokens (const std::vector<std::string_view> token_vector, std::string_view joint, bool treat_as_lyrics_line)
{
    std::string out;
    // To keep timestamps tight together
    bool dont_write_joint = false;

    for (int i = 0; i < token_vector.size(); i++) {
        // First item safeguard
        if (i <= 0) {
            out += token_vector[i]; 
            continue;
        }

        // Lyric lines need their own special treatment. So,
        // If the previous was an opening tag character
        // or the current is a closing one...
        // don't write a space AT THIS EXACT POSITION
        if (
            (
                token_vector[i - 1] == "[" || token_vector[i] == "]"
            ||  token_vector[i - 1] == "<" || token_vector[i] == ">"
            ||  token_vector[i]     == ":"
            ) && treat_as_lyrics_line
        )
            dont_write_joint = true;
        else
            dont_write_joint = false;

        // Perform the actual joint
        if (!dont_write_joint) out.append(joint);
        out.append(token_vector[i]);
    }

    return out;
}

std::string
serialize_tokens (
    const std::vector<std::string>& token_vector,
    std::string_view joint,
    bool treat_as_lyrics_line
)
{
    std::vector<std::string_view> token_views;
    token_views.reserve(token_vector.size());

    for (const std::string& token : token_vector) {
        token_views.emplace_back(token);
    }

    return serialize_tokens(token_views, joint, treat_as_lyrics_line);
}


/**
* @brief Remove leading and trailing whitespace characters.
*/
std::string
trim_string(std::string s)
{
    // left trim
    s.erase(s.begin(),
            std::find_if_not(s.begin(), s.end(),
                             [](unsigned char c){ return std::isspace(c); }));
    // right trim
    s.erase(std::find_if_not(s.rbegin(), s.rend(),
                             [](unsigned char c){ return std::isspace(c); }).base(),
            s.end());
    return s;
}