#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using __dummy_path__ = fs::path;

namespace syrinc {

using filelines = std::vector<std::string>;
using token = std::string; // make the codebase obvious

struct options {
    bool correctoffset = true; // whether to apply the correction internally
    std::optional<long> overrideoffset = std::nullopt; // any offset tag gets ignored and overriden by this
    bool invertoffset = false; // positive offset advances instead of delaying if true
    bool dropmetadata = false; // all tags get dropped
    bool unwrap = false; // multiple timestamp lines get flattened to exactly 1 timestamp per line
};

}