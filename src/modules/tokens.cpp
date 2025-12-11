/**
* @file tokens.cpp
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

#include "modules/timestampconv.hpp"
#include "modules/tokens.hpp"

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
* @brief Find and parse tags in a lyric line.
*
* This function looks for [tag:value] expressions in a lyric line
* and returns a map with all the tags found with their values in
* the form of:
*
* [
*    {
*       .first: "tagname",
*       .second: "tag value"
*    },
*    {
*       .first: "offset",
*       .second: "750":
*    },
*    {
*       .first: "ti",
*       .second: "Ella"
*    }
* ]
*
*/
std::vector<std::pair<std::string, std::string>>
read_tags_from_line (const std::string source)
{
    // Auxiliary output
    std::vector<std::string> prolly_tags;

    // Actual output
    std::vector<std::pair<std::string, std::string>> found_tags;

    // Currently found tag - this will help us concatenate the full tag
    std::string building_tag;

    // Are we currently inside a tag?
    bool currently_in_tag = false;

    // Extract everything from inside [x] and <y> pairs
    for (auto i : source) {
        switch (i) {
            case '[':
            case '<':
                if (!currently_in_tag) currently_in_tag = true;
                break;

            case ']':
            case '>':
                if (!building_tag.empty()) prolly_tags.push_back(building_tag);
                building_tag.clear();
                
                if (currently_in_tag) currently_in_tag = false;
                break;
            
            default:
                if (currently_in_tag) building_tag += i;
                break;
        }
    }

    if (!building_tag.empty()) prolly_tags.push_back(building_tag);

    // Now perform the divisions
    for (auto i : prolly_tags) {
        // Let timestamps intact
        if (is_it_a_timestamp(i)) {
            found_tags.emplace_back(std::pair("time", i));
            continue;
        }

        // Slice tag at the : character
        std::pair<std::string, std::string> slicen_tag = slice_at_character(i, ':');
        slicen_tag.first = trim_string(slicen_tag.first);
        slicen_tag.second = trim_string(slicen_tag.second);

        // If all the previous was true, directly push it to the found tags
        // cutting from the :        
        found_tags.push_back(slicen_tag);
    }

    return found_tags;
}

/**
* @brief Pop out an .lrc tag with such key
*
* This function looks for the tag with such key, finds its opening
* and closing square brackets, and clips out the content outside of
* the bracket-enclosed space.
*
* @param source the lyric line with the key to remove
* @param key key tag to remove
*
* @return source line without such keyed tag
*/
std::string
pop_tag (std::string source, std::string key) {
    std::vector<std::string> tokenized_source = tokenize_line(source, true);

    unsigned long key_index_in_vector = std::string::npos;
    unsigned long opening_bracket_index = 0;
    unsigned long closing_bracket_index = std::string::npos;

    bool will_need_to_repeat = false;

    /* DEBUG
    for (std::string i : tokenized_source) {
        LOG(i, "a token from pop_tag");
    }
    */
    

    // find in vector
    // start right at 1 because there's no way to
    // have a tag already opened by a [ in index
    // below zero, it's absurd
    for (long i = 0; i < tokenized_source.size(); i++) {
        if (tokenized_source[i].find(key) != std::string::npos) {

            /* DEBUG
            std::string safe = tokenized_source[i];
            if (safe == "[") safe = "OPEN";
            if (safe == "]") safe = "CLOSE";
            LOG("token " + std::to_string(i) + " {" + safe + "} is the actual correct tag marker.");
            */
        
            if (key_index_in_vector == std::string::npos) 
                // the tag is present here
                key_index_in_vector = i;
            else
                // This means that we've been here before
                // so there are multiple tags with this key
                // in this line.
                will_need_to_repeat = true;
        }
        /* DEBUG 
        else {
            
            std::string safe = tokenized_source[i];
            if (safe == "[") safe = "OPEN";
            if (safe == "]") safe = "CLOSE";
            LOG("token " + std::to_string(i) + " {" + safe + "} is not the correct tag marker.");
        }
        */
    }

    // If the key was never found, return as-is
    if (key_index_in_vector == std::string::npos) return source;

    // find left brace
    for (long i = key_index_in_vector; i >= 0; i--) {
        if (tokenized_source[i] == "[") {
            opening_bracket_index = i;
            break;
        }
    }

    // find right brace
    for (long i = key_index_in_vector; i < tokenized_source.size(); i++) {
        if (tokenized_source[i] == "]") {
            closing_bracket_index = i;
            break;
        }
    }

    // If matching brackets were not found
    if (
        // if index 0 is not actually a brace
        opening_bracket_index == 0 && tokenized_source[0] != "["
    ||  closing_bracket_index == std::string::npos
    ||  opening_bracket_index >= tokenized_source.size()
    ||  closing_bracket_index >= tokenized_source.size()
    ||  opening_bracket_index > closing_bracket_index
    ) {
        return source; // as-is
    }
    /* DEBUG
    LOG(std::to_string(key_index_in_vector), "key index in vector");
    LOG(std::to_string(opening_bracket_index), "opening bracket index");
    LOG(std::to_string(closing_bracket_index), "closing bracket index");
    */

    // once the indices are found, we'll clip out
    // whatever is not part of the tag we want to pop

    // part before the opening bracket
    std::vector<std::string> lpart (
        tokenized_source.begin(),
        tokenized_source.begin() + opening_bracket_index);

    std::vector<std::string> rpart (
        tokenized_source.begin() + closing_bracket_index + 1,
        tokenized_source.end()
    );
    // we'll need this to validate what we just did
    std::vector<std::string> thepart (
        tokenized_source.begin() + opening_bracket_index,
        tokenized_source.begin() + closing_bracket_index
    );

    // ONLY pop this if the occurence is actually part of the key
    std::string thepart_serialized = serialize_tokens(thepart);
    int thepart_colon_index = thepart_serialized.find(':');
    if (thepart_colon_index != std::string::npos)
        if (thepart_serialized.substr(0, thepart_colon_index).find(key) == std::string::npos)
            return source;

    // concatenate
    lpart.insert(lpart.end(), rpart.begin(), rpart.end());

    std::string out = serialize_tokens(lpart, " ", true);

    if (will_need_to_repeat) out = pop_tag(out, key);

    return out;
}

/**
* @brief Divide a string into a pair, cutting from the first joint character.
*
* @return A string pair containing the text before the joint character and the text after.
* For instance:
*   "offset: 750" -> {"offset", "750"}
*   "correctoffset" -> {"correctoffset", ""}
*
*/
std::pair<std::string, std::string>
slice_at_character (const std::string source, char joint)
{
    std::pair<std::string, std::string> slicen;
    
    // Find tag marker separation index
    int joint_index = source.find_first_of(joint);

    // If the joint is not found, let the entire tag be the name
    if (joint_index == std::string::npos) {
        slicen.first = source;
        return slicen;
    }

    slicen.first = source.substr(0, joint_index);
    slicen.second = source.substr(joint_index + 1, source.length());

    return slicen;
}

/**
* @brief Remove leading and trailing whitespace.
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