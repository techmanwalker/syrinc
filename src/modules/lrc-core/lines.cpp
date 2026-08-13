/**
* @file line.cpp
* 
* @brief Foundation utilities to be able to analyze a lyric string
* looking for timestamps and being able to correct them.
*
* @par correct_line_offset("[00:13.75] A lyric which should appear at second 15", -1250, false);
* @par apply_offset_to_timestamp("00:12.33", -670");
*/

#include <string>
#include <string_view>
#include <vector>

#include "lines.hpp"
#include "timestamps.hpp"
#include "tokens.hpp"

namespace syrinc {
namespace lines {

using namespace syrinc::timestamps;
using namespace syrinc::tokens;

/**
* @brief Correct all the timestamps present in the line
* by applying an offset.
*
* By default, a negative integer delays the timestamp and vice versa
* according to most implementations in the wild, as if a negative
* value meant "this is X milliseconds behind where it's supposed
* to be".
* 
* You can invert this behavior by setting invert_direction to true,
* so it fits with the analogy of a positive number meaning "this
* is intended to be shown X milliseconds later".
* @code
* // returns "[00:15.00] A lyric which should appear at second 15"
* std::string corrected_line = correct_line_offset("[00:13.75] A lyric which should appear at second 15", -1250, false);
* @endcode
*
* @param source the line whose timestamps need to be corrected
* @param offset offset time, expressed in milliseconds
* @param invert_direction negate the sign of the offset
*/
std::string
correct_line_offset (const std::string &source, const long offset, bool invert_direction)
{
    // We will overwrite on the fly and probably
    // we will probably accidentally format the line.

    std::vector<timestamp> timestamps_in_this_line = line_timestamps(source);

    std::string output_line;

    for (timestamp &ts : timestamps_in_this_line) {
            output_line += "[" + ts
                .apply_offset(offset, invert_direction)
                .as_string() + "]";
    }
    
    output_line += " " + trim_string(strip_timestamps(source));

    return output_line;
}

/**
* @brief Return a vector with all of the timestamps that this lyric line contains.
*/
std::vector<timestamp>
line_timestamps (const std::string &source)
{
    std::vector<std::string_view> line_tokens_views = tokenize_line(source, true);
    std::vector<timestamp> timestamps_in_this_line;

    for (std::string_view token : line_tokens_views) {
        if (is_it_a_timestamp(token)) {
            timestamps_in_this_line.emplace_back(token);
            continue;
        };
    }

    return timestamps_in_this_line;
}

/**
* @brief Return the source string with no timestamps.
*/
std::string
strip_timestamps (const std::string &source)
{
    std::vector<std::string_view> line_tokens_views = tokenize_line(source, true);

    std::vector<std::string> output_line_tokens;

    for (std::string_view token : line_tokens_views) {
        if (!is_it_a_timestamp(token)) {
            output_line_tokens.emplace_back(token);
        }
    }

    std::string serialized = serialize_tokens(output_line_tokens, " ", false);

    // remove empty brackets where a timestamp used to be
    replace_all(serialized, "[ ]", "");

    return serialized;
}

}
}