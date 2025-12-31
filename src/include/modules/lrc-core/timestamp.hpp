#pragma once

#include <cstdint>
#include <string>

/**
* Just as a representation helper, this is not
* meant to be directly used anymore
*/
struct ts_components {
    bool is_negative;
    unsigned long mm;
    unsigned long ss;
    unsigned long cs;
};

class timestamp {
    private:
        int64_t duration;

    public:
        timestamp(int64_t duration = 0);
        timestamp(ts_components ts);
        timestamp(std::string source, bool disable_warning = false);
        timestamp(std::string_view source, bool disable_warning = false);

        long
        as_ms() const;

        std::string
        as_string (bool no_filling = false) const;

        ts_components
        as_tsmap (bool zero_negative_timestamps = false) const;

        timestamp &
        apply_offset (const long offset = 0, bool invert_direction = false);
};

int64_t
parse_timestamp (std::string_view source, bool disable_warning = false);

bool
is_it_a_timestamp (const std::string_view source);

bool
is_numeric_only (const std::string_view source);

long
to_long(std::string_view sv);