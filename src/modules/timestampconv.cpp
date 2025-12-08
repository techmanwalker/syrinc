/**
* @file timestampconv.cpp
* @brief Convert timestamps from and to milliseconds.
* @par divide_timestamp("23:24.35");
* @par timestamp_to_ms("23:24.35");
* @par ms_to_timestamp(1404350);
*/

#include "globals.hpp"
#include <cctype>
#include "modules/timestampconv.hpp"

// #include "debug.hpp"

/**
* @brief Separate the timestamp in mm:ss.ms
*
* @param source timestamp to be split.
*
* @code
* // returns mm = 0, ss = 0, ms = 100
* tsmap ts = divide_timestamp("00:00.10");
* 
* // returns mm = 12, ss = 34, ms = 560
* tsmap ts = divide_timestamp("12:34.56");
* @endcode
*
* @return the source timestamp as a map with
* mm, ss and ms members representing the
* original source duration.
*/
tsmap
divide_timestamp (const std::string source)
{
    tsmap ts;

    if (!is_it_a_timestamp(source)) return ts;

    ts["mm"] = ts["ss"] = ts["ms"] = 0;

    int colon_pos = source.find_first_of(':');
    int dot_pos = source.find_first_of('.');

    std::string minutes_component = source.substr(0, colon_pos);
    std::string seconds_component = source.substr(colon_pos + 1, source.length() - dot_pos - 1);
    std::string milliseconds_component = source.substr(dot_pos + 1, source.length() - dot_pos - 1);

    try {
        ts["mm"] = std::stol(minutes_component);
        ts["ss"] = std::stol(seconds_component);
        ts["ms"] = std::stol(milliseconds_component) * 10; // centiseconds → milliseconds
    } catch (...) {
        ts.clear();        // any conversion failure → return empty map
    }
    // the milliseconds component is actually centiseconds, not ms

    return ts;
}

long
timestamp_to_ms (const std::string source)
{
    if (!is_it_a_timestamp(source)) return 0;
    
    tsmap ts = divide_timestamp(source);

    // Convert all the units to ms and sum them together

    return
        ts["mm"] * 60000
    +   ts["ss"] * 1000
    +   ts["ms"];
}

/**
* @brief Convert milliseconds to timestamps.
*
* @code
* // returns 05:10.00
* long ms = ms_to_timestamp(310000);
* @endcode
*
* @param source a time duration expressed in milliseconds
* @param no_filling do not pad the resulting string with leading zeroes
*/
std::string
ms_to_timestamp (const long source, bool no_filling)
{
    tsmap ts;
    long remaining_ms = source;
    bool is_source_negative = false;
    if (remaining_ms < 0) {
        remaining_ms *= -1;
        is_source_negative = true;
    }

    // Progressive reduction, bigger units first

    ts["mm"] = remaining_ms / 60000; remaining_ms -= ts["mm"] * 60000;
    ts["ss"] = remaining_ms / 1000; remaining_ms -= ts["ss"] * 1000;
    ts["ms"] = remaining_ms / 10;

    return // with filling if needed
        std::string(is_source_negative ? "-" : "")
    +   (ts["mm"] < 10 ? "0" : "") + std::to_string(ts["mm"]) + ":"
    +   (ts["ss"] < 10 ? "0" : "") + std::to_string(ts["ss"]) + "."
    +   (ts["ms"] < 10 ? "0" : "") + std::to_string(ts["ms"]);
}

/**
* @brief Check if a given string is an mm:ss.ms timestamp
*
* Ensure that all the chacarters of the string are actually a timestamp representation.
* The only requirements are that all of the characters are digits, : or .
*/
bool
is_it_a_timestamp (const std::string source)
{
    if (source.empty()) return false;
    int colon_count = 0;
    int dot_count = 0;

    for (char i : source) {
        if (
            ! (
                std::isdigit(i)
            ||  i == ':'
            ||  i == '.'
            )

            || (
                dot_count > colon_count
            )

            || (
                dot_count > 1
            ||  colon_count > 1
            )
        )
            return false;

        if (i == ':') colon_count++;
        if (i == '.') dot_count++;
    }

    return colon_count == 1 && dot_count == 1;
}

/**
* @brief Check if a string contains numbers only.
*/
bool
is_numeric_only (const std::string source)
{
    // for "" it returns true as it can be treated as 0
    for (int i = 0; i < source.length(); i++) {
        if (!std::isdigit(source[i]) && source[i] != '.') {
            // Allow the first char to be a minus sign
            if (i == 0 && source[i] == '-') {
                continue;
            }
            else {
                return false;
            }
        }
    }

    return true;
}