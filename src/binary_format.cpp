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

namespace ais2203 {

std::vector<std::uint8_t> encodeBinary(const SensorData& data)
{
    // TODO(Part 3)
    (void)data;
    return {};
}

SensorData decodeBinary(std::span<const std::uint8_t> bytes)
{
    // TODO(Part 3)
    (void)bytes;
    return {};
}

} // namespace ais2203
