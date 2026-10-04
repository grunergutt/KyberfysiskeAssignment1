// Part 3 — the binary format, checked against files a Python script wrote.

#include "ais2203/binary_format.hpp"
#include "test_support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

using namespace ais2203;
using namespace test_support;

TEST_CASE("the header is laid out as the table says", "[part3]")
{
    const std::vector<std::uint8_t> bytes = encodeBinary(expectedReading());
    REQUIRE(bytes.size() >= 19);

    // magic 'A' 'I' 'S' '2', version 1
    CHECK(bytes[0] == 0x41);
    CHECK(bytes[1] == 0x49);
    CHECK(bytes[2] == 0x53);
    CHECK(bytes[3] == 0x32);
    CHECK(bytes[4] == 1);
    // flags: imu valid | accelerometer valid | has temperature
    CHECK(bytes[5] == 0b0000'0111);
    // sequence 10, little-endian: the LOW byte comes first
    CHECK(bytes[6] == 0x0A);
    CHECK(bytes[7] == 0x00);
    CHECK(bytes[8] == 0x00);
    CHECK(bytes[9] == 0x00);
    // length of "imu-north-01"
    CHECK(bytes[18] == 12);
}

TEST_CASE("the size is 67 + N, or 75 + N with a temperature", "[part3]")
{
    CHECK(encodeBinary(expectedReading()).size() == 75 + 12);
    CHECK(encodeBinary(expectedAwkwardReading()).size() == 67 + 8);
    CHECK(encodeBinary(SensorData{}).size() == 67);
}

// The real test. data/*.bin were written by tools/make_golden.py, which has
// never seen your struct. Same bytes = two languages agree on a format.
TEST_CASE("encodeBinary produces exactly the bytes Python produced", "[part3]")
{
    const auto golden = readBytes(dataFile("reading.bin"));
    REQUIRE(golden.size() == 87);
    CHECK(encodeBinary(expectedReading()) == golden);

    const auto goldenAwkward = readBytes(dataFile("reading_no_temperature.bin"));
    REQUIRE(goldenAwkward.size() == 75);
    CHECK(encodeBinary(expectedAwkwardReading()) == goldenAwkward);
}

TEST_CASE("decodeBinary reads the files Python wrote", "[part3]")
{
    CHECK(decodeBinary(readBytes(dataFile("reading.bin"))) == expectedReading());
    CHECK(decodeBinary(readBytes(dataFile("reading_no_temperature.bin"))) == expectedAwkwardReading());
}

TEST_CASE("binary round trip, including values text formats struggle with", "[part3]")
{
    SensorData original = expectedAwkwardReading();
    original.sensor_id = std::string("nul\0inside", 10);   // any byte is fine in a length-prefixed string
    original.sequence = std::numeric_limits<std::uint32_t>::max();
    original.timestamp_ms = std::numeric_limits<std::int64_t>::min();
    original.imu = {true, std::numeric_limits<double>::infinity(), -0.0, std::numeric_limits<double>::denorm_min()};
    original.temperature = std::numeric_limits<double>::lowest();

    const auto bytes = encodeBinary(original);
    REQUIRE_FALSE(bytes.empty());
    const SensorData back = decodeBinary(bytes);

    CHECK(back == original);
    CHECK(std::signbit(back.imu.pitch));   // -0.0 == 0.0, so check the sign bit itself
}

TEST_CASE("a sensor_id that does not fit is refused, not truncated", "[part3]")
{
    SensorData d = expectedReading();
    d.sensor_id = std::string(255, 'x');
    CHECK(encodeBinary(d).size() == 75 + 255);

    d.sensor_id = std::string(256, 'x');
    CHECK_THROWS_AS(encodeBinary(d), std::length_error);
}

// The decoder's input is whatever arrived. EVERY prefix of a valid message is
// an invalid message, and none of them may make the decoder read outside its
// input — run this test with -DLAB_SANITIZER=address to see whether yours does.
TEST_CASE("every truncation of a valid message is a ParseError", "[part3]")
{
    const auto golden = readBytes(dataFile("reading.bin"));
    REQUIRE(golden.size() == 87);

    for (std::size_t length = 0; length < golden.size(); ++length) {
        INFO("first " << length << " byte(s) only");
        // An exact-size copy, so that one byte too far is outside the allocation.
        const std::vector<std::uint8_t> truncated(golden.begin(), golden.begin() + static_cast<std::ptrdiff_t>(length));
        CHECK_THROWS_AS(decodeBinary(truncated), ParseError);
    }
}

TEST_CASE("a sensor_id length that lies is a ParseError", "[part3]")
{
    auto bytes = readBytes(dataFile("reading.bin"));
    REQUIRE(bytes.size() == 87);
    bytes[18] = 255;   // claims 255 characters; the whole message is 87 bytes
    CHECK_THROWS_AS(decodeBinary(bytes), ParseError);
}

TEST_CASE("wrong magic, version, flags or trailing bytes are ParseErrors", "[part3]")
{
    const auto golden = readBytes(dataFile("reading.bin"));
    REQUIRE(golden.size() == 87);

    SECTION("magic")
    {
        auto bytes = golden;
        bytes[3] = '3';
        CHECK_THROWS_AS(decodeBinary(bytes), ParseError);
    }
    SECTION("version")
    {
        auto bytes = golden;
        bytes[4] = 2;
        CHECK_THROWS_AS(decodeBinary(bytes), ParseError);
    }
    SECTION("reserved flag bits")
    {
        auto bytes = golden;
        bytes[5] |= 0x80;
        CHECK_THROWS_AS(decodeBinary(bytes), ParseError);
    }
    SECTION("trailing bytes")
    {
        auto bytes = golden;
        bytes.push_back(0);
        CHECK_THROWS_AS(decodeBinary(bytes), ParseError);
    }
    SECTION("the temperature flag cleared, but the 8 bytes still there")
    {
        auto bytes = golden;
        bytes[5] &= static_cast<std::uint8_t>(~0x04);
        CHECK_THROWS_AS(decodeBinary(bytes), ParseError);
    }
}
