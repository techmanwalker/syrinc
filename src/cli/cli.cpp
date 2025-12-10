#include <cxxopts.hpp>
#include <iostream>
#include <string>

#include "modules/lrcfilerewriter.hpp"
#include "modules/timestampconv.hpp"
#include "modules/tokens.hpp"

int main(int argc, char** argv)
{
    cxxopts::Options opt("syrinc", "LRC offset fixer");

    opt.add_options()
        ("f,file",      "input .lrc file",  cxxopts::value<std::string>())
        ("o,offset", "override offset, in ms", cxxopts::value<std::vector<std::string>>())
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
    long        offset = 0; // will retrieve just next
    bool        invert = result["invert"].as<bool>();

    if (result.count("offset")) {
        auto v = result["offset"].as<std::vector<std::string>>();
        if (v.empty() || v[0].empty() || !is_numeric_only(v[0]))
            offset = std::stol(v[0]);
    }

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