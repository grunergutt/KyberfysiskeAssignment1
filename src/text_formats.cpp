// Parts 1 and 2 — SensorData as JSON and YAML. See include/ais2203/text_formats.hpp
// for what each function must do, and README.md for hints.
//
// The two libraries are already fetched and linked by CMakeLists.txt:
//
//   #include <nlohmann/json.hpp>     https://json.nlohmann.me
//   #include <yaml-cpp/yaml.h>       https://github.com/jbeder/yaml-cpp/wiki/Tutorial
//
// Include them HERE, not in the header.

#include "ais2203/text_formats.hpp"

namespace ais2203 {

std::optional<std::string> readFile(const std::filesystem::path& path)
{
    // TODO(Part 1): the whole file as a string, or std::nullopt if it cannot
    // be opened.
    (void)path;
    return std::nullopt;
}

SensorData parseJson(const std::string& text)
{
    // TODO(Part 1): parse `text` with nlohmann/json. Required fields, one
    // optional field, and EVERY failure leaves as ais2203::ParseError.
    (void)text;
    return {};
}

SensorData parseYaml(const std::string& text)
{
    // TODO(Part 1): the same, with yaml-cpp.
    (void)text;
    return {};
}

std::string toJson(const SensorData& data)
{
    // TODO(Part 2)
    (void)data;
    return {};
}

std::string toYaml(const SensorData& data)
{
    // TODO(Part 2)
    (void)data;
    return {};
}

} // namespace ais2203
