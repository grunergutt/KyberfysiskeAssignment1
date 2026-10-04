// Part 2 — write it back, and prove the round trip.

#include "ais2203/text_formats.hpp"
#include "test_support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <limits>

using namespace ais2203;
using namespace test_support;

TEST_CASE("JSON round trip is exact", "[part2]")
{
    const SensorData original = expectedReading();
    const std::string text = toJson(original);
    INFO("toJson produced:\n" << text);
    REQUIRE_FALSE(text.empty());
    CHECK(parseJson(text) == original);
}

TEST_CASE("YAML round trip is exact", "[part2]")
{
    const SensorData original = expectedReading();
    const std::string text = toYaml(original);
    INFO("toYaml produced:\n" << text);
    REQUIRE_FALSE(text.empty());
    CHECK(parseYaml(text) == original);
}

// The round trip must be exact for EVERY double, not just the friendly ones.
// A writer that prints six significant digits turns 0.1 + 0.2 into 0.3, and
// the test below is red. 17 significant digits (max_digits10) is always
// enough; "the shortest text that parses back to the same double" is nicer.
TEST_CASE("doubles survive text with every bit intact", "[part2]")
{
    SensorData original = expectedAwkwardReading();
    original.imu = {true, 1.0 / 3.0, -2.0 / 3.0, 3.141592653589793};
    original.accelerometer.z = std::numeric_limits<double>::min();   // 2.2250738585072014e-308
    original.temperature = -273.15;

    const std::string json = toJson(original);
    const std::string yaml = toYaml(original);
    INFO("toJson produced:\n" << json);
    INFO("toYaml produced:\n" << yaml);
    REQUIRE_FALSE(json.empty());
    REQUIRE_FALSE(yaml.empty());
    CHECK(parseJson(json) == original);
    CHECK(parseYaml(yaml) == original);
}

TEST_CASE("an empty optional is left out, not written as a fake value", "[part2]")
{
    const SensorData original = expectedAwkwardReading();
    REQUIRE_FALSE(original.temperature.has_value());

    const std::string json = toJson(original);
    const std::string yaml = toYaml(original);
    CHECK(json.find("temperature") == std::string::npos);
    CHECK(yaml.find("temperature") == std::string::npos);
    REQUIRE_FALSE(json.empty());
    REQUIRE_FALSE(yaml.empty());
    CHECK_FALSE(parseJson(json).temperature.has_value());
    CHECK_FALSE(parseYaml(yaml).temperature.has_value());
}

// Strings are where hand-written serializers die: quotes, backslashes,
// colons, a leading '#', non-ASCII. A library gets these right for you — if
// you let it, rather than gluing strings together yourself.
TEST_CASE("awkward sensor ids survive both formats", "[part2]")
{
    for (const char* id : {"plain", "with space", "quote\"inside", "back\\slash", "colon: and # hash",
                           "true", "12345", "", "bl\xC3\xA5skjell"}) {
        INFO("sensor_id: [" << id << "]");
        SensorData original = expectedReading();
        original.sensor_id = id;

        const std::string json = toJson(original);
        const std::string yaml = toYaml(original);
        REQUIRE_FALSE(json.empty());
        REQUIRE_FALSE(yaml.empty());
        CHECK(parseJson(json).sensor_id == original.sensor_id);
        CHECK(parseYaml(yaml).sensor_id == original.sensor_id);
    }
}

TEST_CASE("across formats: JSON in, YAML out, and back", "[part2]")
{
    const auto text = readFile(dataFile("reading.json"));
    REQUIRE(text.has_value());

    const SensorData fromJson = parseJson(*text);
    const std::string yaml = toYaml(fromJson);
    REQUIRE_FALSE(yaml.empty());
    const SensorData fromYaml = parseYaml(yaml);
    const std::string json = toJson(fromYaml);
    REQUIRE_FALSE(json.empty());

    CHECK(parseJson(json) == expectedReading());
}
