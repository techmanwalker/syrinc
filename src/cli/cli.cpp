#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "debug.hpp"
#include "globals.hpp"
#include "metadata.hpp"
#include "process.hpp"
#include "token.hpp"

// Utilities

std::string
parse_options (
    long offset,
    bool invert,
    bool dropmetadata
) {
    return
        "correctoffset"
        // Allow the -o option to override whatever offset the file has
        + (offset != 0 ? ":" + std::to_string(offset) : "") + " "
        + (invert ? "invertoffset" : "") + " "
        + (dropmetadata ? "dropmetadata" : "");
}

fs::path
build_temp_name (const fs::path source, std::string temp_signature)
{
    return (fs::temp_directory_path() / source.stem()).string() + temp_signature + source.extension().string();
}

int
atomic_write_lrc_file (
    const fs::path save_as,
    filelines tokens
)
{
    // Create output parent directory before attempting anything 
    if (!save_as.parent_path().empty()
    &&  !fs::exists(save_as.parent_path())
    )
        fs::create_directories(save_as.parent_path());

    // Attach a -temp before the extension
    fs::path temporary_filename = build_temp_name(save_as, "-temp");

    // write to temporary file
    std::ofstream out(temporary_filename);
    out << 
        serialize_tokens(
            tokens, "\n"
        );
    
    out.close();

    // perform atomic write
    try {
        if (fs::exists(save_as)) fs::remove(save_as);
        fs::copy(temporary_filename, save_as);
        fs::remove(temporary_filename);
    } catch (const fs::filesystem_error &e) {
        std::cerr << "Failed to write output .lrc file: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

filelines
read_lines_from_stdin ()
{
    filelines feed;
    // read stdin line by line
    std::string line;
    while (std::getline(std::cin, line))          // blocks until pipe closes
        feed.push_back(std::move(line));
    if (!std::cin.eof() && std::cin.bad()) {       // real I/O error
        std::cerr << "i/o error: couldn't read stdin";
        return feed;
    }

    return feed;
}

std::string
which(const std::string &program)
{
    std::string cmd = trim_string(program);

    // Take only the first word up to the first space
    size_t space_pos = cmd.find(' ');
    if (space_pos != std::string::npos) {
        cmd = cmd.substr(0, space_pos);
    }

    const char *pathEnv = std::getenv("PATH");
    if (!pathEnv) return "";

    std::stringstream ss(pathEnv);
    std::string dir;
    while (std::getline(ss, dir, ':')) {
        std::string candidate = dir + "/" + cmd;
        if (::access(candidate.c_str(), X_OK) == 0) {
            return candidate;
        }
    }

    return "";
}

int
handle_lrc_file_directly (
    fs::path file,
    fs::path save_as,
    long offset,
    bool offset_provided,
    bool invert,
    bool dropmetadata
) {

    // Allow reading from file
    bool use_stdin = file == "-";

    // options that will be fed to the process_lyrics engine

    std::string options = parse_options(offset, invert, dropmetadata);

    // For debugging
    LOG(options, "Processing lyrics with following options");

    // Fire a warning if the user manually typed offset 0
    if (offset_provided && offset == 0)
        std::clog << "warning: -o 0 means \"use file offset\"; "
                "file offset will be used.\n";

    // to simplify code reading, we'll save the processed lyrics here
    filelines processed_lyrics_tokens;

    // Allow reading from stdin
    if (use_stdin) {
        // Read .lrc data from stdin
        filelines feed = read_lines_from_stdin();
        processed_lyrics_tokens = process_lyrics(feed, options);
    } else if (!file.empty()) {
        processed_lyrics_tokens = process_lyrics(file, options);
    }

    // Warn about empty file
    if (processed_lyrics_tokens.size() == 0)
        std::cerr << "Input audio file had no lyrics metadata." << std::endl;

    if (save_as.empty()) {
        // write to stdout
        std::cout <<
            serialize_tokens(
                processed_lyrics_tokens, "\n"
            )
        << std::endl;
    } else {
        atomic_write_lrc_file(save_as, processed_lyrics_tokens);
    }

    return 0;
}

int
handle_audio_file_directly (
    fs::path audio_file,
    fs::path save_as,
    long offset,
    bool offset_provided,
    bool invert,
    filelines source_lyrics // allows to override the lyrics inst
)
{
    // Ensure 100% format compatibility while still
    // allowing writing to stdout
    if (
        !save_as.empty()
    &&  audio_file.extension() != save_as.extension()
    &&  save_as.extension() != ".lrc"
    &&  save_as != "-"
    ) {
        std::cerr << "Source and destination extension must be the same, except for exporting an .lrc file." << std::endl;
        return 1;
    }

    // When working directly with audio metadata files, metadata MUST be dropped
    // to avoid showing up in the player
    std::string options = parse_options(offset, invert, true);

    // Fire a warning if the user manually typed offset 0
    if (offset_provided && offset == 0)
        std::clog << "WARNING: -o 0 means \"use file offset\"; "
                "file offset will be used.\n";

    // to simplify code reading, we'll save the processed lyrics here
    filelines processed_lyrics_tokens;

    // Feed the lyrics to process_lyrics
    processed_lyrics_tokens = process_lyrics(source_lyrics, options);

    // Warn about empty file
    if (processed_lyrics_tokens.size() == 0)
        std::cerr << "Input audio file had no lyrics metadata." << std::endl;

    if (save_as.extension() != ".lrc") {
        if (save_as.empty() || save_as == "-") {
            // write to stdout
            std::cout <<
                serialize_tokens(
                    processed_lyrics_tokens, "\n"
                )
            << std::endl;
        }
        else {
            // Check if ffmpeg is installed
            if (which("ffmpeg").empty()) {
                std::cerr << "FFmpeg needs to be installed and be present in $PATH to save directly as audio.\nEither install FFmpeg or save to an external .lrc file."  << std::endl;
                return 1;
            }

            // Create output parent directory before attempting anything 
            if (!fs::exists(save_as.parent_path())) fs::create_directories(save_as.parent_path());

            // perform atomic write
            try {
                fs::path temporary_filename = build_temp_name(save_as, "-temp");
                change_metadata_field_value(
                    audio_file,
                    temporary_filename,
                    "LYRICS",
                    processed_lyrics_tokens
                );

                if (fs::exists(save_as)) fs::remove(save_as);
                fs::copy(temporary_filename, save_as.string());
                fs::remove(temporary_filename);
            } catch (const fs::filesystem_error &e) {
                std::cerr << "Failed to write output audio file: " << e.what() << std::endl;
                return 1;
            }
        }
    } else {
        atomic_write_lrc_file(save_as, processed_lyrics_tokens);
    }

    return 0;
}

int main(int argc, char** argv)
{
    cxxopts::Options opt("syrinc", ".lrc offset fixer");

    opt.add_options()
        ("f,file",      "input .lrc or song file, - to read from stdin",  cxxopts::value<std::string>())
        ("l,link-lrc", "override audio's .lrc metadata with an external file instead", cxxopts::value<std::string>())
        ("s,save-as", "Save output to path "
                                 "   (leave empty for stdout, type :in: to overwrite source file)", cxxopts::value<std::string>()->default_value(""))
        ("o,offset", "override offset, in ms", cxxopts::value<long>())
        ("i,invert",    "invert offset sign")
        ("d,drop-metadata", "Drop out lyrics metadata tags")
        ("h,help",      "print full help");

    const char* examples = R"(
Examples:
  Embed .lrc file into the audio metadata in place
    syrinc -f audio.flac -l lyrics.lrc -s :in:

  Export audio lyrics metadata to an external .lrc file
    syrinc -f audio.flac -s lyrics.lrc

  Correct audio lyrics timestamps and overwrite the source file
    syrinc -f audio.flac -s :in:

  Correct timestamps and save to another path
    syrinc -f audio.flac -s output.flac

  Apply custom offset and overwrite source file
    syrinc -f audio.flac -o 500 -s :in:

  Correct timestamps with time offset - standalone .lrc
    syrinc -f lyrics.lrc -s :in:
)";

    try {
        auto result = opt.parse(argc, argv);
        if (result.count("help")) {
            std::cout << opt.help() << '\n'
                << examples << std::endl;
            return 0;
        }
        if (!result.count("file"))     // -f missing
            throw cxxopts::exceptions::missing_argument("file");

        /* ----- JSON-like retrieval ----- */
        std::string file           = result["file"].as<std::string>();
        // allow link-lrc to be empty
        std::string link_lrc = result.count("link-lrc")
            ? result["link-lrc"].as<std::string>()
            : "";
        std::string save_as        = result["save-as"].as<std::string>();
        bool        invert         = result["invert"].as<bool>();
        bool        dropmetadata   = result["drop-metadata"].as<bool>();

        // Respect in-place overwrite
        if (save_as == ":in:") save_as = file;

        // otherwise treat as an .lrc file
        bool treat_as_audio =
            // explicitly stated that it's not an .lrc file 
            fs::path(file).extension() != ".lrc"
            // and not trying to read from stdin
        &&  file != "-";

        // retrieve offset like this so we can detect if the user typed -o 0 by accident
        long raw_offset = result.count("offset")
                    ? result["offset"].as<long>()
                    : LONG_MIN;   // your sentinel
        bool offset_provided = raw_offset != LONG_MIN;
        long offset = offset_provided ? raw_offset : 0;

        // Return if file doesn't even exist 
        // AND if the user didn't meant that it's a file (like reading stdin)
        if (file != "-" && !fs::exists(file)) {
            std::cerr << "File \"" << file << "\" does not exist." << std::endl;
            return 1;
        }

        if (file == "-" && treat_as_audio) {
            std::cerr << "Reading audio files via stdin is not supported. Use -f instead." << std::endl;
            return 1;
        }

        // Treat file as...
        if (treat_as_audio) {
            return handle_audio_file_directly(
                file,
                save_as,
                offset,
                offset_provided,
                invert,
                // by default, just take whatever the audio metadata has
                (link_lrc.empty() ? get_audio_lyrics(file) : 
                    // else, process the external .lrc instead first
                    process_lyrics(
                        link_lrc,
                        parse_options(offset, invert, 
                        true // always drop metadata when dealing with audio files
                        )
                    )
                )
            );
        } else {
            if (!link_lrc.empty())
                std::cout << "warning: both input files are .lrc, ignoring link-lrc input..." << std::endl;
            return handle_lrc_file_directly(
                file,
                save_as,
                offset,
                offset_provided,
                invert,
                dropmetadata
            );
        }

    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error: " << e.what() << "\n\n";
        std::cout << opt.help() << '\n'
                << examples << std::endl;
        return 1;
    }

    return 0;
}