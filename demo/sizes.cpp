// Not a test — a measurement. Run it once Parts 1, 2 and 3 are green and copy
// the table into ANSWERS.md (question 1).
//
// "Binary is smaller" is the kind of claim you should measure before you
// believe it. How much smaller, for THIS message? And where do the bytes go?

#include "ais2203/binary_format.hpp"
#include "ais2203/text_formats.hpp"

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>

using namespace ais2203;

namespace {

void row(const std::string& what, std::size_t bytes, std::size_t baseline)
{
    std::cout << "  " << std::left << std::setw(36) << what << std::right << std::setw(6) << bytes << " bytes";
    if (baseline != 0) {
        std::cout << std::setw(8) << std::fixed << std::setprecision(0)
                  << 100.0 * static_cast<double>(bytes) / static_cast<double>(baseline) << " %";
    }
    std::cout << "\n";
}

// The same JSON with the whitespace between tokens removed (whitespace inside
// a string is data, and stays).
std::string withoutWhitespace(const std::string& json)
{
    std::string out;
    bool inString = false;
    for (std::size_t i = 0; i < json.size(); ++i) {
        const char c = json[i];
        if (c == '"' && (i == 0 || json[i - 1] != '\\')) {
            inString = !inString;
        }
        if (inString || (c != ' ' && c != '\n' && c != '\r' && c != '\t')) {
            out += c;
        }
    }
    return out;
}

} // namespace

int main()
{
    const auto text = readFile(std::string(DATA_DIR) + "/reading.json");
    if (!text) {
        std::cerr << "cannot read data/reading.json — is Part 1 done?\n";
        return 1;
    }

    try {
        const SensorData reading = parseJson(*text);

        const std::string json = toJson(reading);
        const std::string compact = withoutWhitespace(json);
        const std::string yaml = toYaml(reading);
        const auto binary = encodeBinary(reading);

        std::cout << "One SensorData (sensor_id \"" << reading.sensor_id << "\", with temperature):\n\n";
        row("toJson(), as you format it", json.size(), compact.size());
        row("toJson(), whitespace removed", compact.size(), compact.size());
        row("toYaml()", yaml.size(), compact.size());
        row("encodeBinary()", binary.size(), compact.size());

        std::cout << "\nAt 50 Hz: " << compact.size() * 50 << " B/s as compact JSON, " << binary.size() * 50
                  << " B/s as binary.\n";
    } catch (const std::exception& e) {
        std::cerr << "failed: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
