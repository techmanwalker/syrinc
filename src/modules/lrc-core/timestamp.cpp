/**
* @file timestamp.cpp
* @brief Convert timestamps from and to milliseconds.
* @par divide_timestamp("23:24.35");
* @par timestamp_to_ms("23:24.35");
* @par ms_to_timestamp(1404350);
*/

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>

#include "timestamp.hpp"


/*
* Allow the same timestamp to be represented in its
* multiple forms
*/

// Construct a timestamp from an already-ms length
timestamp::timestamp(int64_t duration)
{
    this->duration = duration;
}

/**
* @brief Construct a timestamp from a string by
* separating the timestamp in mm:ss.cs
* to mm, ss, ms format.
*
* @param source timestamp to be split.
*
* @code
* // returns mm = 0, ss = 0, ms = 100
* ts_components ts = timestamp("00:00.10").as_tsmap();
* 
* // returns mm = 12, ss = 34, ms = 560
* ts_components ts = divide_timestamp("12:34.56").as_tsmap();
* @endcode
*
* @return the source timestamp as a struct with
* mm, ss and cs members representing the
* original source duration.
*/
int64_t
parse_timestamp (std::string source, bool disable_warning)
{
    if (!is_it_a_timestamp(source)) {
        return 0;
    };

    ts_components ts;

    // if a round-trip is performed, we'll advise the user
    // at the end
    bool trigger_warning = false;

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
    // that will automatically balance the lengths

    if (
        ts.ss >= 60
    ||  ts.cs >= 100
    ) {
        ts = timestamp(
            (ts.mm * 60000)
            + (ts.ss * 1000)
            + (ts.cs * 10)
        ).as_tsmap();

        trigger_warning = true;
    }

    int64_t duration = ((ts.mm * 60000)
                    + (ts.ss * 1000)
                    + (ts.cs * 10))
                    * (is_negative ? -1 : 1);

    // Trigger a warning if a roundtrip had to be performed
    if (trigger_warning && !disable_warning)
            // obviously will show a warning
            std::cerr << "warning: " + source + " timestamp is malformed; will round up to " + timestamp(duration).as_string() + "..." << std::endl;

    return duration;
}

timestamp::timestamp (std::string source, bool disable_warning)
{
    this->duration = parse_timestamp(source, disable_warning);
}

/**
* @brief Return timestamp as a milliseconds length.
*/
long
timestamp::as_ms() const
{
    return this->duration;
}

/**
* @brief Return the timestamp as a timestamp map form.
*
* @code
* // returns mm = 0, ss = 0, ms = 100
* ts_components ts = divide_timestamp("00:00.10");
* 
* // returns mm = 12, ss = 34, ms = 560
* ts_components ts = divide_timestamp("12:34.56");
* @endcode
*
* @param zero_negative_timestamp clamp all negative timestamps
* to zero
*
* @return the source timestamp as a struct with
* mm, ss and cs members representing the timestamp
*/
ts_components
timestamp::as_tsmap (bool zero_negative_timestamps) const
{
    ts_components ts;
    ts.mm = ts.ss = ts.cs = 0;
    long remaining_ms = this->duration;
    bool is_source_negative = false;
    if (remaining_ms < 0) {
        // Round negative timestamps up to zero if needed
        if (!zero_negative_timestamps) {
            remaining_ms *= -1;
            is_source_negative = true;
        } else
            // just return a zeroed timestamp, no need to
            // manually convert
            return ts;
    }

    // Progressive reduction, bigger units first

    ts.mm = remaining_ms / 60000; remaining_ms -= ts.mm * 60000;
    ts.ss = remaining_ms / 1000; remaining_ms -= ts.ss * 1000;
    ts.cs = remaining_ms / 10;

    return ts;
}

std::string
timestamp::as_string (bool no_filling) const
{
    ts_components ts = this->as_tsmap();

    return // with filling if needed
        std::string((this->duration < 0) ? "-" : "")
    +   (ts.mm < 10 && !no_filling ? "0" : "") + std::to_string(ts.mm) + ":"
    +   (ts.ss < 10 && !no_filling ? "0" : "") + std::to_string(ts.ss) + "."
    +   (ts.cs < 10 && !no_filling ? "0" : "") + std::to_string(ts.cs);
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
timestamp &
timestamp::apply_offset (const long offset, bool invert_direction)
{    
    this->duration -= (offset * (invert_direction ? -1 : 1));

    // prevent from going below zero
    if (this->duration <= 0) this->duration = 0;

    return *this;
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