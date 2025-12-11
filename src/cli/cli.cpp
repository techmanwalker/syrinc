#include <cxxopts.hpp>
#include <iostream>
#include <string>

#include "debug.hpp"
#include "modules/lrcfilerewriter.hpp"
#include "modules/tokens.hpp"

int main(int argc, char** argv)
{
    cxxopts::Options opt("syrinc", "LRC offset fixer");

    opt.add_options()
        ("f,file",      "input .lrc file",  cxxopts::value<std::string>())
        ("o,offset", "override offset, in ms",
        cxxopts::value<long>()->default_value(std::to_string(LONG_MIN))) // sentinel value
        ("i,invert",    "invert offset sign")
        ("d,drop-metadata", "Drop out all metadata tags")
        ("h,help",      "print usage");

    auto result = opt.parse(argc, argv);

    if (result.count("help") || !result.count("file"))
    {
        std::cout << opt.help() << '\n';
        return 0;
    }

    /* ----- JSON-like retrieval ----- */
    std::string file         = result["file"].as<std::string>();
    bool        invert       = result["invert"].as<bool>();
    bool        dropmetadata = result["drop-metadata"].as<bool>();

    // retrieve offset like this so we can detect if the user typed -o 0 by accident
    long raw_offset = result["offset"].as<long>();
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

    if (!file.empty())
        std::cout <<
            serialize_tokens(
                process_lyrics(
                    file,
                    options
                ),
                "\n",
                false
            )
        << std::endl;
        
}