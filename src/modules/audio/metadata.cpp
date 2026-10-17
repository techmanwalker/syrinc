#include <filesystem>
#include <string_view>

#include "globals.hpp"
#include "tokens.hpp"
#include "metadata.hpp"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/pixfmt.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/dict.h>
}

namespace syrinc {
namespace audio {

using namespace syrinc::tokens;

namespace {

struct format_ctx_closer {
    void operator()(AVFormatContext *ctx) const { avformat_close_input(&ctx); }
};
using format_ctx_ptr = std::unique_ptr<AVFormatContext, format_ctx_closer>;

// Case-insensitive prefix match on "lyrics" (LYRICS, lyrics, lyrics-eng, ...).
// Container tags first (FLAC, MP3, M4A, MKV...), then per-stream tags (Ogg: Opus, Vorbis).
const char *
find_lyrics_value (const AVFormatContext *fmt)
{
    constexpr int flags = AV_DICT_IGNORE_SUFFIX;

    if (const auto *e = av_dict_get(fmt->metadata, "lyrics", nullptr, flags))
        return e->value;

    for (unsigned i = 0; i < fmt->nb_streams; ++i) {
        if (const auto *e = av_dict_get(fmt->streams[i]->metadata, "lyrics", nullptr, flags))
            return e->value;
    }

    return nullptr;
}

} // namespace

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

    // 1. Open file/container. The explicit "file:" stops names like "a:b.opus"
    //    from being parsed as protocol URLs.
    const std::string uri = "file:" + url.string();
    AVFormatContext *raw_fmt = nullptr;
    if (avformat_open_input(&raw_fmt, uri.c_str(), nullptr, nullptr) < 0)
        return feed;                       // silent: there are no lyrics
    const format_ctx_ptr fmt(raw_fmt);     // closed on every exit path

    // 2. read header
    avformat_find_stream_info(fmt.get(), nullptr);

    // 3. Get metadata dictionary
    const char *value = find_lyrics_value(fmt.get());
    if (value) {
        // FFmpeg give us the WHOLE lrc block in a single string
        // Slice by line jump \n
        std::string_view lyrics(value);
        size_t pos = 0;
        while (pos < lyrics.size()) {
            size_t end = lyrics.find('\n', pos);
            if (end == std::string_view::npos) end = lyrics.size();
            feed.emplace_back(lyrics.substr(pos, end - pos));
            pos = end + 1;
        }
    }

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

}
}