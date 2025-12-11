#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "debug.hpp"
#include "modules/lrcfilerewriter.hpp"
#include "modules/tokens.hpp"

int main(int argc, char** argv)
{
    cxxopts::Options opt("syrinc", "LRC offset fixer");

    opt.add_options()
        ("f,file",      "input .lrc file, - to read from stdin",  cxxopts::value<std::string>())
        ("o,offset", "override offset, in ms", cxxopts::value<long>())
        ("i,invert",    "invert offset sign")
        ("d,drop-metadata", "Drop out lyrics metadata tags")
        ("h,help",      "print usage");

    try {
        auto result = opt.parse(argc, argv);
        if (result.count("help")) {
            std::cout << opt.help() << '\n';
            return 0;
        }
        if (!result.count("file"))     // -f missing
            throw cxxopts::exceptions::missing_argument("file");

        /* ----- JSON-like retrieval ----- */
        std::string file         = result["file"].as<std::string>();
        bool        invert       = result["invert"].as<bool>();
        bool        dropmetadata = result["drop-metadata"].as<bool>();

        // Allow reading from file
        bool use_stdin = file == "-";

        // retrieve offset like this so we can detect if the user typed -o 0 by accident
        long raw_offset = result.count("offset")
                    ? result["offset"].as<long>()
                    : LONG_MIN;   // your sentinel
        bool offset_provided = raw_offset != LONG_MIN;
        long offset = offset_provided ? raw_offset : 0;

        // options that will be fed to the process_lyrics engine

        std::string options = "correctoffset"
            // Allow the -o option to override whatever offset the file has
            + (offset != 0 ? ":" + std::to_string(offset) : "") + " "
            + (invert ? "invertoffset" : "") + " "
            + (dropmetadata ? "dropmetadata" : "");

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
            filelines feed;
            
            // read stdin line by line
            std::string line;
            while (std::getline(std::cin, line))          // blocks until pipe closes
                feed.push_back(std::move(line));
            if (!std::cin.eof() && std::cin.bad()) {       // real I/O error
                std::cerr << "i/o error: couldn't read stdin";
                return 1;
            }


            processed_lyrics_tokens = process_lyrics(feed, options);
        } else if (!file.empty()) {
            processed_lyrics_tokens = process_lyrics(file, options);
        }

        std::cout <<
            serialize_tokens(
                processed_lyrics_tokens, "\n"
            )
        << std::endl;
            
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "error: " << e.what() << "\n\n";
        std::cout << opt.help() << '\n';
        return 1;
    }

    return 0;
}