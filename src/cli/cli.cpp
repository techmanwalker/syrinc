#include <cxxopts.hpp>
#include <iostream>
#include <string>

#include "../include/modules/lrcfilerewriter.hpp"
#include "../include/modules/tokens.hpp"

int main(int argc, char** argv)
{
    cxxopts::Options opt("syrinc", "LRC offset fixer");

    opt.add_options()
        ("f,file",      "input .lrc file",  cxxopts::value<std::string>())
        ("o,offset",    "override offset, in ms",     cxxopts::value<long>()->default_value("0"))
        ("i,invert",    "invert offset sign")
        ("h,help",      "print usage");

    auto result = opt.parse(argc, argv);

    if (result.count("help") || !result.count("file"))
    {
        std::cout << opt.help() << '\n';
        return 0;
    }

    /* ----- JSON-like retrieval ----- */
    std::string file   = result["file"].as<std::string>();
    long        offset = result["offset"].as<long>();
    bool        invert = result["invert"].as<bool>();

    if (!file.empty())
        std::cout <<
            serialize_tokens(
                process_lyrics(
                    file,
                    
                    // Allow for offset correction
                    std::string("correctoffset")

                    // Allow the -o option to override whatever offset the file has
                    + (offset > 0 ? ":" + std::to_string(offset) : "") + " "
                    + (invert ? "invertoffset" : "")
                ),
                "\n",
                false
            )
        << std::endl;
        
}