/**
* @file tokens.cpp
* @brief Tools to divide a lyric line in a series of tokens and
* serialize afterwards.
* @par tokenize_lyric_line ("[00:00:00] Beginning of a song");
* @par serialize_lyric_tokens ({"[", "00:12.34", "Another", "part"});
* 
*/

#include <string>
#include <vector>

#include "../include/modules/tokens.hpp"

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
std::vector<std::string>
tokenize_lyric_line (const std::string source)
{
    std::vector<std::string> tokens;
    std::string current_token;

    // By default, separate solely for spaces.
    for (int i = 0; i < source.length(); i++) {
        if (source[i] == ' ' && !current_token.empty()) {
            tokens.push_back(current_token);
            current_token = "";
        }
        
        // If it is a tag character...
        if (
            (
                source[i] == '[' || source[i] == ']'
            ||  source[i] == '<' || source[i] == '>'
            )
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
serialize_lyric_tokens (const std::vector<std::string> token_vector)
{
    std::string out;
    // To keep timestamps tight together
    bool dont_write_space = false;

    for (int i = 0; i < token_vector.size(); i++) {
        // First item safeguard
        if (i <= 0) {
            out += token_vector[i]; 
            continue;
        }

        // If the previous was an opening tag character
        // or the current is a closing one...
        if (
            (
                token_vector[i - 1] == "[" || token_vector[i] == "]"
            ||  token_vector[i - 1] == "<" || token_vector[i] == ">"
            )
        )
            dont_write_space = true;
        else
            dont_write_space = false;

        // Perform the actual joint
        out += (!dont_write_space ? " " : "") + token_vector[i];
    }

    return out;
}