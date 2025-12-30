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
std::vector<token>
tokenize_line (const std::string source, bool treat_as_lyrics_line)
{
    std::vector<std::string> tokens;
    std::string current_token;

    // By default, separate solely for spaces.
    for (int i = 0; i < source.length(); i++) {
        if (source[i] == ' ' && !current_token.empty()) {
            tokens.push_back(current_token);
            current_token = "";
        }
        
        // Lyric lines need their own special treatment. So,
        // if it is a tag character...
        if (
            (
                source[i] == '[' || source[i] == ']'
            ||  source[i] == '<' || source[i] == '>'
            ) && treat_as_lyrics_line
        ) {
            // For the sake of emptiness
            if (!current_token.empty())
                tokens.push_back(current_token);

            // Push back this tag character SEPARATELY
            tokens.emplace_back(1, source[i]);

            // Clean buffer
            current_token = "";
        } else {
            if (source[i] != ' ')
                current_token += source[i];
        }
    }

    if (!current_token.empty()) tokens.push_back(current_token);
    
    return tokens;
}

/**
* @brief Convert a token listt to a single string
*
* Just concatenate a token vector back to a string, but ensuring that 
* timestamp tags stay together like [00:00.00] and don't become sparse
* like [ 00:00:00 ] or < 00:00:00 >
*
* @param source the token vector to be converted back to string
*
*/
std::string
serialize_tokens (const std::vector<std::string> token_vector, std::string joint, bool treat_as_lyrics_line)
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
        out += (!dont_write_joint ? joint : "") + token_vector[i];
    }

    return out;
}

/**
* @brief Remove leading and trailing whitespace characters.
*/
std::string trim_string(std::string s)
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