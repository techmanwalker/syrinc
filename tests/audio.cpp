#include <iostream>
#include <string>

#include "metadata.hpp"

/* ---------- get_audio_lyrics (FFmpeg-C API) ---------- */
void TEST_get_audio_lyrics(const char *url)
{
    std::cout << "\n===== get_audio_lyrics =====\n";

    // Feed the test asset that lives in tests/audio1.flac
    // const char *url = "audio1.flac";
    filelines lines = get_audio_lyrics(url);

    if (lines.empty()) {
        std::cerr << "No LYRICS tag found in " << url << "\n";
        return;
    }

    std::cout << "LYRICS block (" << lines.size() << " lines):\n";
    for (std::size_t i = 0; i < lines.size(); ++i)
        // std::cout << "  [" << i << "] " << lines[i] << "\n";
        std::cout << lines[i] << "\n";
}

void TEST_change_metadata_field_value (const char *url)
{
    std::cout << 
        change_metadata_field_value(
            url, 
            url + std::string("-modified.flac"), 
            "LYRICS", 
            "[00:05.00] Test lyrics")
        << std::endl;
}

int main(int argc, char **argv) {
    TEST_get_audio_lyrics(argv[1]);
    TEST_change_metadata_field_value(argv[1]);
    TEST_get_audio_lyrics((std::string(argv[1]) + std::string("-modified.flac")).c_str());
}