#pragma once

#include "../../globals.hpp"

filelines
get_audio_lyrics(const fs::path source);

std::string
change_metadata_field_value(
    const std::filesystem::path &source,
    const std::filesystem::path &output,
    const std::string_view field_name,
    const std::string_view field_value
);

std::string
change_metadata_field_value (
    const fs::path &source,
    const fs::path &output,
    const std::string_view field_name,
    const filelines field_value
);