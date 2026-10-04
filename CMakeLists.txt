# Compulsory assignment 1 — build description. Same shape as the labs:
#
#   1. Three libraries are downloaded and built the first time you configure
#      (internet needed, a few minutes): nlohmann/json, yaml-cpp and Catch2.
#      vcpkg is week 41 — until then FetchContent does the job.
#   2. The code you write is ONE library, `telemetry` (src/*.cpp). The tests
#      and the demo both link it — which is why it is a library at all.
#   3. Each tests/test_*.cpp becomes one program AND one registered test, so
#      `ctest` (and CI) can tell you which parts are red.

cmake_minimum_required(VERSION 3.16)
project(assignment1_serialization CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# Archives fetched by URL get the extraction time as their timestamp (the
# modern default; this only silences a warning on CMake >= 3.24).
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# Optional: build everything with a sanitizer (GCC/Clang — on Windows that is
# your WSL toolchain from the debugging lab).
#   -DLAB_SANITIZER=address,undefined   Part 3: does your decoder read past its input?
# The flags go on before the libraries are fetched, so everything is
# instrumented.
set(LAB_SANITIZER "" CACHE STRING "Sanitizer(s) to enable, e.g. address,undefined")
if(LAB_SANITIZER AND NOT MSVC)
    add_compile_options(-fsanitize=${LAB_SANITIZER} -g -O1 -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${LAB_SANITIZER})
endif()

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        yaml-cpp-0.9.0
    GIT_SHALLOW    TRUE)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
    GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(json yaml-cpp Catch2)

function(assignment_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif()
endfunction()

# ---- the code you write -----------------------------------------------------
add_library(telemetry STATIC
    src/text_formats.cpp
    src/binary_format.cpp)
target_include_directories(telemetry PUBLIC include)
# PRIVATE: the JSON and YAML libraries are used inside src/ only — no header in
# include/ mentions them, so nothing that links `telemetry` needs to know they
# exist.
target_link_libraries(telemetry PRIVATE nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp)
assignment_warnings(telemetry)

# ---- tests --------------------------------------------------------------------
enable_testing()

set(ASSIGNMENT_TESTS
    test_01_parse
    test_02_write
    test_03_binary)

foreach(test ${ASSIGNMENT_TESTS})
    add_executable(${test} tests/${test}.cpp)
    target_include_directories(${test} PRIVATE tests)
    target_link_libraries(${test} PRIVATE telemetry Catch2::Catch2WithMain)
    target_compile_definitions(${test} PRIVATE DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
    assignment_warnings(${test})

    add_test(NAME ${test} COMMAND ${test})
    set_tests_properties(${test} PROPERTIES TIMEOUT 120)
endforeach()

# ---- demo: run it by hand once Parts 1-3 are green (ANSWERS.md, question 1) ----
add_executable(sizes demo/sizes.cpp)
target_link_libraries(sizes PRIVATE telemetry)
target_compile_definitions(sizes PRIVATE DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
assignment_warnings(sizes)
