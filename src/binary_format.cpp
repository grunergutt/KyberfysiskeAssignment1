// Part 3 — the binary format. The layout table is in
// include/ais2203/binary_format.hpp; tools/make_golden.py is the same layout
// written in Python.
//
// Two rules, both from the lecture:
//   1. Write VALUES, not memory. No memcpy of the struct, no reinterpret_cast
//      of a pointer to a number — shifts, one byte at a time, in the order the
//      table says. (For a double: std::bit_cast<std::uint64_t>(d), header
//      <bit>, gives you its 64 bits as an integer you can shift.)
//   2. On the way in, check that the bytes are THERE before you read them.
//      Decide where that check lives before you write the first read.

#include "ais2203/binary_format.hpp"

#include <bit>          // std::bit_cast
#include <cstdint>
#include <stdexcept>    // std::length_error
#include <vector>

namespace ais2203 {

namespace {   // helpers: only visible inside this .cpp file

// Write a 32-bit number as 4 bytes, lowest byte first (little-endian).
void putU32le(std::vector<std::uint8_t>& out, std::uint32_t v)
{
    out.push_back(static_cast<std::uint8_t>(v));
    out.push_back(static_cast<std::uint8_t>(v >> 8));
    out.push_back(static_cast<std::uint8_t>(v >> 16));
    out.push_back(static_cast<std::uint8_t>(v >> 24));
}

// Write a 64-bit number as 8 bytes, lowest byte first (little-endian).
void putU64le(std::vector<std::uint8_t>& out, std::uint64_t v)
{
    out.push_back(static_cast<std::uint8_t>(v));
    out.push_back(static_cast<std::uint8_t>(v >> 8));
    out.push_back(static_cast<std::uint8_t>(v >> 16));
    out.push_back(static_cast<std::uint8_t>(v >> 24));
    out.push_back(static_cast<std::uint8_t>(v >> 32));
    out.push_back(static_cast<std::uint8_t>(v >> 40));
    out.push_back(static_cast<std::uint8_t>(v >> 48));
    out.push_back(static_cast<std::uint8_t>(v >> 56));
}

// Write a double as its 64 bits, little-endian.
void putF64le(std::vector<std::uint8_t>& out, double d)
{
    putU64le(out, std::bit_cast<std::uint64_t>(d));
}

    struct Reader {
    std::span<const std::uint8_t> bytes;
    std::size_t pos = 0;

    void need(std::size_t n) const
    {
        if (bytes.size() - pos < n) {
            throw ParseError("binary reading is too short");
        }
    }

    std::uint8_t u8()
    {
        need(1);
        return bytes[pos++];
    }

    std::uint32_t u32le()
    {
        need(4);
        std::uint32_t v = std::uint32_t{bytes[pos]}
        | (std::uint32_t{bytes[pos + 1]} << 8)
        | (std::uint32_t{bytes[pos + 2]} << 16)
        | (std::uint32_t{bytes[pos + 3]} << 24);
        pos += 4;
        return v;
    }

    std::uint64_t u64le()
    {
        need(8);
        std::uint64_t v = 0;
        for (int i = 0; i < 8; ++i) {
            v |= std::uint64_t{bytes[pos + i]} << (8 * i);
        }
        pos += 8;
        return v;
    }

    double f64le()
    {
        return std::bit_cast<double>(u64le());
    }
};

} 

std::vector<std::uint8_t> encodeBinary(const SensorData& data)
{
    // The length must fit in one byte. Refuse rather than silently cut it.
    if (data.sensor_id.size() > 255) {
        throw std::length_error("sensor_id is longer than 255 bytes");
    }

    std::vector<std::uint8_t> out;

    // offset 0, 4 bytes: magic "AIS2"
    out.push_back('A');
    out.push_back('I');
    out.push_back('S');
    out.push_back('2');

    // offset 4, 1 byte: version
    out.push_back(1);

    // offset 5, 1 byte: flags
    std::uint8_t flags = 0;
    if (data.imu.is_valid)           { flags |= 0b001; }   // bit 0
    if (data.accelerometer.is_valid) { flags |= 0b010; }   // bit 1
    if (data.temperature)            { flags |= 0b100; }   // bit 2
    out.push_back(flags);

    // offset 6, 4 bytes: sequence
    putU32le(out, data.sequence);

    // offset 10, 8 bytes: timestamp (signed, written as its raw 64 bits)
    putU64le(out, static_cast<std::uint64_t>(data.timestamp_ms));

    // offset 18, 1 byte: N = length of sensor_id
    out.push_back(static_cast<std::uint8_t>(data.sensor_id.size()));

    // offset 19, N bytes: the characters, no terminator
    for (char c : data.sensor_id) {
        out.push_back(static_cast<std::uint8_t>(c));
    }

    // offset 19+N, 24 bytes: imu roll, pitch, yaw
    putF64le(out, data.imu.roll);
    putF64le(out, data.imu.pitch);
    putF64le(out, data.imu.yaw);

    // offset 43+N, 24 bytes: accelerometer x, y, z
    putF64le(out, data.accelerometer.x);
    putF64le(out, data.accelerometer.y);
    putF64le(out, data.accelerometer.z);

    // offset 67+N, 8 bytes: temperature, ONLY if present
    if (data.temperature) {
        putF64le(out, *data.temperature);
    }

    return out;
}

SensorData decodeBinary(std::span<const std::uint8_t> bytes)
{
    Reader r{bytes};
    SensorData data;

    if (r.u8() != 'A' || r.u8() != 'I' || r.u8() != 'S' || r.u8() != '2') {
        throw ParseError("wrong magic, not an AIS2 reading");
    }

    std::uint8_t version = r.u8();
    if (version != 1) {
        throw ParseError("unknown version");
    }

    std::uint8_t flags = r.u8();          // keep it, you need it later
    if (flags & 0b11111000) {             // any of bits 3-7 on?
        throw ParseError("reserved flag bits are set");
    }

    data.sequence = r.u32le();
    data.timestamp_ms = static_cast<std::int64_t>(r.u64le());

    std::uint8_t n = r.u8();              // how many characters the name has

    // sensor_id: n characters
    for (std::uint8_t i = 0; i < n; ++i) {
        data.sensor_id.push_back(static_cast<char>(r.u8()));
    }

    // imu: 3 doubles
    data.imu.roll  = r.f64le();
    data.imu.pitch = r.f64le();
    data.imu.yaw   = r.f64le();
    data.imu.is_valid = (flags & 0b001) != 0;

    // accelerometer: 3 doubles
    data.accelerometer.x = r.f64le();
    data.accelerometer.y = r.f64le();
    data.accelerometer.z = r.f64le();
    data.accelerometer.is_valid = (flags & 0b010) != 0;

    // temperature: only if flag bit 2 is on
    if (flags & 0b100) {
        data.temperature = r.f64le();
    }

    // the bookmark must be exactly at the end: nothing left over
    if (r.pos != bytes.size()) {
        throw ParseError("extra bytes after the reading");
    }

    return data;
}

} // namespace ais2203
