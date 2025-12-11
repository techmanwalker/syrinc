cmake_minimum_required(VERSION 3.10)
project(syrinc)

set(CMAKE_CXX_STANDARD 20) # Set the desired C++ standard (e.g., 11, 14, 17, 20, 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Ensure the compiler supports the specified standard
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CXXOPTS REQUIRED cxxopts>=2.0)

# ---- default BUILD_TESTS to ON when Debug ----
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT DEFINED BUILD_TESTS)
  set(BUILD_TESTS ON CACHE BOOL "Build test executable" FORCE)
endif()

# this one contains the code to process .lrc files such as
# applying the offset values and partial formatting
file(GLOB_RECURSE lrc-core-cpp
    src/modules/lrc-core/*.cpp
)
add_library(lrc-core STATIC
    "${lrc-core-cpp}"
)
target_include_directories(lrc-core PUBLIC
    "${CMAKE_SOURCE_DIR}/src/include"
    "${CMAKE_SOURCE_DIR}/src/include/modules/lrc-core"
)
# activate or deactivate the debug logging macro
target_compile_definitions(lrc-core PUBLIC
    $<$<CONFIG:Debug>:DEBUG_BUILD>
)

# build the basic cli interface
add_executable(syrinc
    src/cli/cli.cpp
)
target_link_libraries(syrinc PRIVATE lrc-core)
target_include_directories(syrinc PUBLIC
    "${CMAKE_SOURCE_DIR}/src/include"
)

# only if requested with -DBUILD_TESTS
if(BUILD_TESTS)
  add_executable(test-lrc-core "tests/lrc-core.cpp")
  target_link_libraries(test-lrc-core PRIVATE lrc-core)
  target_include_directories(test-lrc-core PUBLIC
    "${CMAKE_SOURCE_DIR}/src/include"
  )
endif()