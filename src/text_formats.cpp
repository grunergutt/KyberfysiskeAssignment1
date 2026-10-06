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
    json j;
    j["sensor_id"] = data.sensor_id;
    j["sequence"] = data.sequence;
    j["timestamp_ms"] = data.timestamp_ms;
    j["imu"]["is_valid"] = data.imu.is_valid;
    j["imu"]["roll"] = data.imu.roll;
    j["imu"]["pitch"] = data.imu.pitch;
    j["imu"]["yaw"] = data.imu.yaw;
    j["accelerometer"]["is_valid"] = data.accelerometer.is_valid;
    j["accelerometer"]["x"] = data.accelerometer.x;
    j["accelerometer"]["y"] = data.accelerometer.y;
    j["accelerometer"]["z"] = data.accelerometer.z;
    if (data.temperature)
    {
        j["temperature"] = *data.temperature;   // the double inside
    }
    return j.dump();
}

std::string toYaml(const SensorData& data)
{
    // TODO(Part 2)
    YAML::Node root;
    root["sensor_id"] = data.sensor_id;
    root["sequence"] = data.sequence;
    root["timestamp_ms"] = data.timestamp_ms;
    root["imu"]["is_valid"] = data.imu.is_valid;
    root["imu"]["roll"] = data.imu.roll;
    root["imu"]["pitch"] = data.imu.pitch;
    root["imu"]["yaw"] = data.imu.yaw;
    root["accelerometer"]["is_valid"] = data.accelerometer.is_valid;
    root["accelerometer"]["x"] = data.accelerometer.x;
    root["accelerometer"]["y"] = data.accelerometer.y;
    root["accelerometer"]["z"] = data.accelerometer.z;
    if (data.temperature)
    {
        root["temperature"] = *data.temperature;
    }
    YAML::Emitter out;
    out << root;          // hand the whole node to the emitter
    return out.c_str();

    //ELLER
    // Alternative: the same output, written with YAML::Emitter.
// The Emitter streams the structure in order: BeginMap / Key / Value / EndMap.
// Compared with building a YAML::Node it gives more control over formatting
// (flow vs block style, quoting, comments, number precision) and skips the
// intermediate tree. For this assignment the Node version is simpler.
//
// std::string toYaml(const SensorData& data)
// {
//     YAML::Emitter out;
//     out.SetDoublePrecision(17);   // 17 significant digits: any double survives the round trip
//
//     out << YAML::BeginMap;                                                   // outer map
//     out << YAML::Key << "sensor_id"    << YAML::Value << data.sensor_id;
//     out << YAML::Key << "sequence"     << YAML::Value << data.sequence;
//     out << YAML::Key << "timestamp_ms" << YAML::Value << data.timestamp_ms;
//
//     out << YAML::Key << "imu" << YAML::Value;                                // value of "imu" is...
//     out << YAML::BeginMap;                                                   // ...a nested map
//     out << YAML::Key << "is_valid" << YAML::Value << data.imu.is_valid;
//     out << YAML::Key << "roll"     << YAML::Value << data.imu.roll;
//     out << YAML::Key << "pitch"    << YAML::Value << data.imu.pitch;
//     out << YAML::Key << "yaw"      << YAML::Value << data.imu.yaw;
//     out << YAML::EndMap;                                                     // end of imu
//
//     out << YAML::Key << "accelerometer" << YAML::Value;
//     out << YAML::BeginMap;
//     out << YAML::Key << "is_valid" << YAML::Value << data.accelerometer.is_valid;
//     out << YAML::Key << "x"        << YAML::Value << data.accelerometer.x;
//     out << YAML::Key << "y"        << YAML::Value << data.accelerometer.y;
//     out << YAML::Key << "z"        << YAML::Value << data.accelerometer.z;
//     out << YAML::EndMap;                                                     // end of accelerometer
//
//     if (data.temperature)                                                    // left out when empty
//     {
//         out << YAML::Key << "temperature" << YAML::Value << *data.temperature;
//     }
//
//     out << YAML::EndMap;                                                     // end of outer map
//     return out.c_str();
// }
}

} // namespace ais2203
