#pragma once
/**
* @brief Type representing a timestamp and its components.
*
* In this program, it consists of only
* mm, ss and ms members to represent a song
* timestamp.
*
* @note "cs" means "the centesimal part of a second", hence
* 1 cs = 10 ms
*
*/

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using __dummy_path__ = fs::path;

using filelines = std::vector<std::string>;
using token = std::string; // make the codebase obvious