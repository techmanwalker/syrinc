/**
* @file lrcfilerewriter.cpp
* @brief The actual .lrc data rewriter.
*
* Here we'll take a plugins-like approach where the developer can
* stack multiple functions a.k.a. processing passes for each line.
* This way, one can run posprocessing passes besides the simple
* offset correction if it's ever needed.
*
* Contains some file I/O management code.
*
*/

#include <filesystem>
#include <fstream>
#include <utility>
#include <vector>

#include "modules/correctlineoffset.hpp"
#include "modules/lrcfilerewriter.hpp"
#include "modules/timestampconv.hpp"
#include "modules/tokens.hpp"

/**
* @brief Perform required processing steps to lyrics metadata.
*
* This function takes a vector of strings which corresponds to
* a sequences of lines in the .lrc file format for lyrics.
*
* Available options:
*   - correctoffset: Find and the [offset: ms] tag and apply the
*     offset to all the timestamps found after the tag.
*     For every [offset] tag found, the timestamps after it will
*     be compensated against it. For example, 00:12.45 with an
*     offset of 750 negative will delay the timestamp by 750 ms,
*     resulting in the lyric being shown at 00:13.70. You can
*     invert the direction of the offset with the invertoffset
*     option.
*     This option accepts an integer value expressed in ms, which
*     overrides whatever offset value the file itself contains.
*   - invertoffset: Invert the sign of the offset. By default,
*     a negative offset actually delays the timestamp rather than
*     advancing it to show up sooner. This is because the way
*     most players interpret the offset sign. With this option,
*     you will compensate the timestamps in the inverted order:
*     a positive offset will delay the time where a lyric is
*     shown and a negative will advance it.
*
* @param lyrics A vector containing lyrics lines, preferably
* read from a .lrc file, but there's a direct overload to read
* directly data from an .lrc file.
* @param options Processing options explained above, expressed
* in a string with space-separated tokens like "option1 option2"
*/
filelines
process_lyrics (const filelines lyrics, const std::string options)
{
    filelines out;

    // Read the options string
    std::vector<std::string> options_tokens = tokenize_line(options);

    // Cherry-pick the actually supported options

    bool correctoffset = false;
    bool overrideoffset = false;
    bool invertoffset = false;

    // Placeholder variables
    long offset = 0;

    // Traverse through the tokenized options
    for (std::string o : options_tokens) {
        // opair = option pair key, value
        std::pair<std::string, std::string> opair = slice_at_character(o, ':');
        // trim pair, just in case
        opair.first = trim_string(opair.first);
        opair.second = trim_string(opair.second);

        if (opair.first == "correctoffset") {
            correctoffset = true;

            // Override only if requested
            if (! (opair.second == "") && is_numeric_only(opair.second)) {
                offset = std::stol(opair.second);
                overrideoffset = true;
            }

            continue;
        }

        if (opair.first == "invertoffset") invertoffset = true;
    }

    // Apply the intended processing steps for each single line
    for (const std::string i : lyrics) {
        // Fist of all, let's gather information from the lines themselves.
        std::vector<std::pair<std::string, std::string>> tags = read_tags_from_line(i);

        // To be able to pop off offset lines
        bool does_this_line_have_an_offset_tag = false;

        // look for an "offset" tag in the current line
        for (const auto& [key, value] : tags)
        {
            if ((key == "offset") || (key == "of"))
            {
                if (!value.empty() && is_numeric_only(value)) {
                    offset = (!overrideoffset ? std::stol(value) : offset);   // update running offset
                }

                // pop off this line
                does_this_line_have_an_offset_tag = true;

                break;                           // first offset wins
            }
        }

        std::string processed_line = i;

        // Don't accidentally take away metadata or lyrics but rather
        // remove the offset tag
        if (does_this_line_have_an_offset_tag) processed_line = pop_tag(processed_line, "of");

        // Pop empty lines as well
        if (trim_string(processed_line) == "") continue;

        if (correctoffset)
            processed_line = correct_line_offset(processed_line, offset, invertoffset);

        out.push_back(processed_line);
    }

    return out;
}

/**
* @brief Perform required processing steps to lyrics metadata.
* 
* @note This is an overload to allow directly reading from an .lrc file
*/
filelines
process_lyrics (const fs::path lyrics, const std::string options)
{
    /* 
        Read the file line by line and just feed it to the original
        function
    */

    std::ifstream lrcfile(lyrics);

    if (!lrcfile.is_open()) return filelines();

    filelines feed;

    std::string line;
    while (std::getline(lrcfile, line)) {
        feed.push_back(line);
    }

    // Feed and return
    return
        process_lyrics(feed, options);
}