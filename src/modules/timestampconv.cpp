/**
* @file timestampconv.cpp
* @brief Convert timestamps from and to milliseconds.
* @par divide_timestamp("23:24.35");
* @par timestamp_to_ms("23:24.35");
* @par ms_to_timestamp(1404350);
*/

#include "globals.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include "modules/timestampconv.hpp"

// #include "debug.hpp"

/**
* @brief Separate the timestamp from mm:ss.cs
* to mm, ss, ms format.
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
* @return the source timestamp as a struct with
* mm, ss and cs members representing the
* original source duration.
*/
tsmap
divide_timestamp (const std::string source, bool disable_warning)
{
    tsmap ts;

    if (!is_it_a_timestamp(source)) return ts;

    ts.mm = ts.ss = ts.cs = 0;

    int colon_pos = source.find_first_of(':');
    int dot_pos = source.find_first_of('.');

    /* is_it_a_timestamp requires : and . to be
    present to pass, so no need to verify it
    again
    */

    bool is_negative = source[0] == '-';

    std::string minutes_component = source.substr((is_negative ? 1 : 0), colon_pos);
    std::string seconds_component = source.substr(colon_pos + 1, source.length() - dot_pos - 1);
    std::string centiseconds_component = source.substr(dot_pos + 1, source.length() - dot_pos - 1);

    // the check of the components being all numeric
    // was already done by is_it_a_timestamp, so
    // no need of a try-catch here
    ts.is_negative = is_negative;
    ts.mm = std::stol(minutes_component);
    ts.ss = std::stol(seconds_component);
    ts.cs = std::stol(centiseconds_component);

    // Everybody could make mistakes with formatting
    // so I'll be forgiving by performing a round-trip
    // that will automatically balance the lenghts

    if (
        ts.ss >= 60
    ||  ts.cs >= 100
    ) {
        std::string roundtrip_ts = ms_to_timestamp(
            (ts.mm * 60000)
            + (ts.ss * 1000)
            + (ts.cs * 10)
        );

        ts = divide_timestamp(roundtrip_ts);

        if (!disable_warning)
            // obviously will show a warning
            std::cerr << "warning: " + source + " timestamp is malformed; will round up to " + roundtrip_ts + "...";
    }

    return ts;
}

long
timestamp_to_ms (const std::string source)
{
    if (!is_it_a_timestamp(source)) return 0;
    
    tsmap ts = divide_timestamp(source);

    // Convert all the units to ms and sum them together

    return
    (
        ts.mm * 60000
    +   ts.ss * 1000
    +   ts.cs * 10
    ) * (ts.is_negative ? -1 : 1);
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
ms_to_timestamp (const long source, bool no_filling, bool zero_negative_timestamps)
{
    tsmap ts;
    ts.mm = ts.ss = ts.cs = 0;
    long remaining_ms = source;
    bool is_source_negative = false;
    if (remaining_ms < 0) {
        // Round negative timestamps up to zero if needed
        if (!zero_negative_timestamps) {
            remaining_ms *= -1;
            is_source_negative = true;
        } else
            // just return a zeroed timestamp, no need to
            // manually convert
            return "00:00.00";
    }

    // Progressive reduction, bigger units first

    ts.mm = remaining_ms / 60000; remaining_ms -= ts.mm * 60000;
    ts.ss = remaining_ms / 1000; remaining_ms -= ts.ss * 1000;
    ts.cs = remaining_ms / 10;

    return // with filling if needed
        std::string(is_source_negative ? "-" : "")
    +   (ts.mm < 10 ? "0" : "") + std::to_string(ts.mm) + ":"
    +   (ts.ss < 10 ? "0" : "") + std::to_string(ts.ss) + "."
    +   (ts.cs < 10 ? "0" : "") + std::to_string(ts.cs);
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

    bool is_negative = source[0] == '-';

    // count colons and dots
    int colon_count = std::count(source.begin(), source.end(), ':');
    int dot_count = std::count(source.begin(), source.end(), '.');


    // It is a malformed timestamp if it doesn't contain
    // exactly 1 : and 1 .
    if (colon_count != 1 || dot_count != 1)
        return false;

    // allowing the first char to be a minus sign
    for (int i = (is_negative ? 1 : 0); i < source.length(); i++) {
        if (
            ! (
                std::isdigit(source[i])
            ||  source[i] == ':'
            ||  source[i] == '.'
            )
        ) {
            return false;
        }
    }

    return true;
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