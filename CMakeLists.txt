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
file(GLOB_RECURSE tsrewind_cpp
    src/modules/*.cpp
)
add_library(tsrewind STATIC
    "${tsrewind_cpp}"
)
target_include_directories(tsrewind PUBLIC
    "${CMAKE_SOURCE_DIR}/src/include"
)
# activate or deactivate the debug logging macro
target_compile_definitions(tsrewind PUBLIC
    $<$<CONFIG:Debug>:DEBUG_BUILD>
)

# build the basic cli interface
add_executable(syrinc
    src/cli/cli.cpp
)
target_link_libraries(syrinc PRIVATE tsrewind)

# only if requested with -DBUILD_TESTS
if(BUILD_TESTS)
  add_executable(test "tests/tests.cpp")
  target_link_libraries(test PRIVATE tsrewind)
  target_include_directories(test PUBLIC
    "${CMAKE_SOURCE_DIR}/src/include"
  )
endif()