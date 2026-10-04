// Part 1 — read a file, parse JSON and YAML, and fail properly on bad input.

#include "ais2203/text_formats.hpp"
#include "test_support.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace ais2203;
using namespace test_support;

TEST_CASE("readFile returns the content, or nullopt for a missing file", "[part1]")
{
    const auto text = readFile(dataFile("reading.json"));
    REQUIRE(text.has_value());
    CHECK(text->find("imu-north-01") != std::string::npos);

    CHECK_FALSE(readFile(dataFile("no_such_file.json")).has_value());
}

TEST_CASE("parseJson reads every field", "[part1]")
{
    const auto text = readFile(dataFile("reading.json"));
    REQUIRE(text.has_value());

    const SensorData d = parseJson(*text);
    const SensorData expected = expectedReading();

    CHECK(d.sensor_id == expected.sensor_id);
    CHECK(d.sequence == expected.sequence);
    CHECK(d.timestamp_ms == expected.timestamp_ms);
    CHECK(d.imu == expected.imu);
    CHECK(d.accelerometer == expected.accelerometer);
    REQUIRE(d.temperature.has_value());
    CHECK(*d.temperature == 21.5);
}

TEST_CASE("parseYaml reads every field", "[part1]")
{
    const auto text = readFile(dataFile("reading.yaml"));
    REQUIRE(text.has_value());

    const SensorData d = parseYaml(*text);
    const SensorData expected = expectedReading();

    CHECK(d.sensor_id == expected.sensor_id);
    CHECK(d.sequence == expected.sequence);
    CHECK(d.timestamp_ms == expected.timestamp_ms);
    CHECK(d.imu == expected.imu);
    CHECK(d.accelerometer == expected.accelerometer);
    REQUIRE(d.temperature.has_value());
    CHECK(*d.temperature == 21.5);
}

TEST_CASE("temperature is optional; awkward values survive", "[part1]")
{
    // sequence 4 000 000 000 needs all 32 unsigned bits, timestamp_ms is
    // negative, and y is 0.1 + 0.2 — which is not 0.3.
    const auto json = readFile(dataFile("reading_no_temperature.json"));
    const auto yaml = readFile(dataFile("reading_no_temperature.yaml"));
    REQUIRE(json.has_value());
    REQUIRE(yaml.has_value());

    CHECK(parseJson(*json) == expectedAwkwardReading());
    CHECK(parseYaml(*yaml) == expectedAwkwardReading());
    CHECK_FALSE(parseJson(*json).temperature.has_value());
}

TEST_CASE("the two formats describe the same value", "[part1]")
{
    const auto json = readFile(dataFile("reading.json"));
    const auto yaml = readFile(dataFile("reading.yaml"));
    REQUIRE(json.has_value());
    REQUIRE(yaml.has_value());
    CHECK(parseJson(*json) == parseYaml(*yaml));
}

// Deserializing is parsing input you do not control. Whatever is wrong with
// it, the caller sees ParseError — not json::parse_error, not
// YAML::BadConversion, and certainly not a default-constructed SensorData
// that looks like a real reading of all zeros.
TEST_CASE("bad JSON becomes ParseError", "[part1]")
{
    for (const char* name : {"broken_truncated.json", "broken_missing_field.json", "broken_wrong_type.json"}) {
        INFO("file: " << name);
        const auto text = readFile(dataFile(name));
        REQUIRE(text.has_value());
        CHECK_THROWS_AS(parseJson(*text), ParseError);
    }
    CHECK_THROWS_AS(parseJson(""), ParseError);
    CHECK_THROWS_AS(parseJson("[1, 2, 3]"), ParseError);   // valid JSON, wrong shape
}

TEST_CASE("bad YAML becomes ParseError", "[part1]")
{
    for (const char* name : {"broken_syntax.yaml", "broken_missing_field.yaml", "broken_wrong_type.yaml"}) {
        INFO("file: " << name);
        const auto text = readFile(dataFile(name));
        REQUIRE(text.has_value());
        CHECK_THROWS_AS(parseYaml(*text), ParseError);
    }
    CHECK_THROWS_AS(parseYaml(""), ParseError);
    CHECK_THROWS_AS(parseYaml("- 1\n- 2\n"), ParseError);   // valid YAML, wrong shape
}
