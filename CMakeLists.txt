cmake_minimum_required(VERSION 3.10)
project(syrinc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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