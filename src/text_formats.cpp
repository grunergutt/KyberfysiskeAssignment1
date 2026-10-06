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
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

using json = nlohmann::json;

namespace ais2203 {

std::optional<std::string> readFile(const std::filesystem::path& path)
{
    // TODO(Part 1): the whole file as a string, or std::nullopt if it cannot
    // be opened.
    std::ifstream file(path, std::ios::binary);
    if (!file){return std::nullopt;}

    std::ostringstream fileContents;
    fileContents << file.rdbuf();
    return fileContents.str();
}

SensorData parseJson(const std::string& text)
{
    // TODO(Part 1): parse `text` with nlohmann/json. Required fields, one
    // optional field, and EVERY failure leaves as ais2203::ParseError.
    try {
        json j = json::parse(text); // 2. parse
        SensorData data;// 3. SensorData data;
        data.sensor_id = j.at("sensor_id").get<std::string>();// 4. sensor_id, sequence, timestamp_ms  (at + get)
        data.sequence = j.at("sequence").get<std::uint32_t>();
        data.timestamp_ms = j.at("timestamp_ms").get<std::int64_t>();
        data.imu.is_valid = j.at("imu").at("is_valid").get<bool>();
        data.imu.roll = j.at("imu").at("roll").get<double>();
        data.imu.pitch = j.at("imu").at("pitch").get<double>();
        data.imu.yaw = j.at("imu").at("yaw").get<double>();
        data.accelerometer.is_valid = j.at("accelerometer").at("is_valid").get<bool>();
        data.accelerometer.x = j.at("accelerometer").at("x").get<double>();
        data.accelerometer.y = j.at("accelerometer").at("y").get<double>();
        data.accelerometer.z = j.at("accelerometer").at("z").get<double>();
        if (j.contains("temperature")){
            data.temperature = j.at("temperature").get<double>();
        }
        return data;
    }
    catch (const json::exception& e) {
        throw ParseError(e.what());
    }
}

SensorData parseYaml(const std::string& text)
{
    // TODO(Part 1): the same, with yaml-cpp.
    try
    {
        YAML::Node root = YAML::Load(text);
        if (!root.IsMap())
        {
            throw ParseError("expected a YAML map at the top level");
        }
        SensorData data;
        data.sensor_id = root["sensor_id"].as<std::string>();
        data.sequence = root["sequence"].as<std::uint32_t>();
        data.timestamp_ms = root["timestamp_ms"].as<std::int64_t>();
        data.imu.is_valid = root["imu"]["is_valid"].as<bool>();
        data.imu.roll = root["imu"]["roll"].as<double>();
        data.imu.pitch = root["imu"]["pitch"].as<double>();
        data.imu.yaw = root["imu"]["yaw"].as<double>();
        data.accelerometer.is_valid = root["accelerometer"]["is_valid"].as<bool>();
        data.accelerometer.x = root["accelerometer"]["x"].as<double>();
        data.accelerometer.y = root["accelerometer"]["y"].as<double>();
        data.accelerometer.z = root["accelerometer"]["z"].as<double>();
        if (root["temperature"])
        {
            data.temperature = root["temperature"].as<double>();
        }
        return data;
    }
    catch (const YAML::Exception& e) {
        throw ParseError(e.what());
    }
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
