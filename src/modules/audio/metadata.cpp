#include <filesystem>
#include <string_view>

#include "globals.hpp"
#include "token.hpp"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/pixfmt.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/dict.h>
}

/**
* @brief Get song lyrics from the LYRICS file metadata field.
*
* This function scrapes the song lyrics with FFmpeg to get the lyrics
* to perform our manual offset correction and processing.
*
* @param url song's location in filesystem
* @return the lyric lines of the song in form of a vector of strings
*/
filelines
get_audio_lyrics(const fs::path url)
{
    filelines feed;
    AVFormatContext *fmt = nullptr;

    // 1. Open file/container
    if (avformat_open_input(&fmt, url.c_str(), nullptr, nullptr) < 0)
        return feed;                       // silent: there are no lyrics

    // 2. read header
    avformat_find_stream_info(fmt, nullptr);

    // 3. Get metadata dictionary
    AVDictionaryEntry *e = av_dict_get(fmt->metadata, "LYRICS", nullptr, 0);
    if (e && e->value) {
        // FFmpeg give us the WHOLE lrc block in a single string
        // Slice by line jump \n
        std::string_view lyrics(e->value);
        size_t pos = 0;
        while (pos < lyrics.size()) {
            size_t end = lyrics.find('\n', pos);
            if (end == std::string_view::npos) end = lyrics.size();
            feed.emplace_back(lyrics.substr(pos, end - pos));
            pos = end + 1;
        }
    }

    // 4. Clean
    avformat_close_input(&fmt);
    return feed;
}

/**
* @brief Write a new audio file with the exact same streams as the
* source but change the metadata field value to the new string.
* 
* @param source original audio file
* @param output output audio file, with no streams changed
* @param field_name metadata field key to change
* @param value new value for such metadata field
*/
std::string
change_metadata_field_value(
    const std::filesystem::path &source,
    const std::filesystem::path &output,
    const std::string_view field_name,
    const std::string_view field_value
)
{
    // Build the ffmpeg command
    std::ostringstream cmd;
    cmd << "ffmpeg -i " 
        << std::quoted(source.string()) 
        << " -c copy"  // Copy all streams without re-encoding
        << " -metadata " << field_name << "=" << std::quoted(field_value)
        << " " << std::quoted(output.string())
        << " -y"  // Overwrite output file
        << " 2>&1";  // Capture stderr
    
    // Execute
    int ret = std::system(cmd.str().c_str());
    
    if (ret != 0) {
        return "ffmpeg failed with code: " + std::to_string(ret);
    }
    
    return "success";
}

/**
* @brief Vectorial overload for the homonym function.
*
* Allows an std::vector<std::string> to be used as a multiline string
* input for the field value parameter.
*
* @param source original audio file
* @param output output audio file, with no streams changed
* @param field_name metadata field key to change
* @param value new value for such metadata field
*/
std::string
change_metadata_field_value (
    const fs::path &source,
    const fs::path &output,
    const std::string_view field_name,
    const filelines field_value
)
{
    std::vector<std::string_view> field_value_views;
    for (int i = 0; i < field_value.size(); i++) {
        field_value_views.emplace_back(std::string_view(field_value[i]));
    }

    return change_metadata_field_value(
        source,
        output,
        field_name,
        serialize_tokens(field_value_views, "\n", false)
    );
}