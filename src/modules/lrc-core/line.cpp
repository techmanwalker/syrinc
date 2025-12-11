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
#include <vector>

#include "line.hpp"
#include "timestamp.hpp"
#include "token.hpp"

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
correct_line_offset (const std::string source, const long offset, bool invert_direction)
{
    // We will overwrite on the fly and probably
    // we will accidentally format the line.

    std::vector<std::string> line_tokens = tokenize_line(source, true);

    // We will solely correct the timestamps here.
    for (int i = 0; i < line_tokens.size(); i++) {
        if (!is_it_a_timestamp(line_tokens[i])) continue;

        line_tokens[i] = apply_offset_to_timestamp(line_tokens[i], offset, invert_direction);
    }

    return serialize_tokens(line_tokens, " ", true);
}

/**
* @brief Apply an offset, expressed in milliseconds, to a timestamp.
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
* // returns "00:13.00"
* std::string ts = apply_offset_to_timestamp("00:12.33", -670");
* @endcode
*
* @param source the timestamp to be corrected
* @param offset offset value, expressed in milliseconds
* @param invert_direction negate the sign of the offset
*/
std::string
apply_offset_to_timestamp (const std::string source, const long offset, bool invert_direction)
{
    if (!is_it_a_timestamp(source)) return source;

    long ts_in_ms = timestamp_to_ms(source);
    
    ts_in_ms -= (offset * (invert_direction ? -1 : 1));

    // prevent from going below zero
    if (ts_in_ms <= 0) ts_in_ms = 0;

    return ms_to_timestamp(ts_in_ms);
}