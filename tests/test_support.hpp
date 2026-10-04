// Helpers shared by the tests. DO NOT EDIT anything in tests/ — CI runs the
// original tests, and so does the person approving your hand-in.

#ifndef AIS2203_TEST_SUPPORT_HPP
#define AIS2203_TEST_SUPPORT_HPP

#include "ais2203/sensor_data.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#ifndef DATA_DIR
#error "DATA_DIR must be defined by CMake"
#endif

namespace test_support {

inline std::filesystem::path dataFile(const std::string& name)
{
    return std::filesystem::path(DATA_DIR) / name;
}

inline std::vector<std::uint8_t> readBytes(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

// What data/reading.json, reading.yaml and reading.bin all say.
inline ais2203::SensorData expectedReading()
{
    ais2203::SensorData d;
    d.sensor_id = "imu-north-01";
    d.sequence = 10;
    d.timestamp_ms = 1790236800123;
    d.imu = {true, 0.14, 0.013, -5.68};
    d.accelerometer = {true, 0.0, -0.01, -0.23};
    d.temperature = 21.5;
    return d;
}

// What the reading_no_temperature.* files say: values chosen to be awkward.
inline ais2203::SensorData expectedAwkwardReading()
{
    ais2203::SensorData d;
    d.sensor_id = "acc-mast";
    d.sequence = 4000000000u;                  // does not fit in a signed 32-bit int
    d.timestamp_ms = -1;                       // negative: two's complement on the wire
    d.imu = {false, 0.0, 0.0, 0.0};
    d.accelerometer = {true, 9.81, 0.1 + 0.2, -1e-9};   // 0.1 + 0.2 is 0.30000000000000004
    return d;                                  // no temperature
}

} // namespace test_support

#endif // AIS2203_TEST_SUPPORT_HPP
