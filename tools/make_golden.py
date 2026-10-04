"""Writes data/reading.bin and data/reading_no_temperature.bin.

This is "the other end": a Python program that knows nothing about your C++
struct — only the layout table in include/ais2203/binary_format.hpp. If your
decodeBinary() reads these files correctly, and your encodeBinary() produces
the same bytes, then two programs in two languages agree on a wire format.
That is the whole point of serialization.

You do not need to run this; the .bin files are already in data/. It is here
so you can read it. Note the '<' in the format strings: struct.pack's way of
saying "little-endian, no padding" — the two things a raw C++ struct cannot
promise.

    python tools/make_golden.py
"""
import json
import pathlib
import struct

DATA = pathlib.Path(__file__).resolve().parent.parent / "data"


def encode(r: dict) -> bytes:
    has_temperature = r.get("temperature") is not None
    flags = (r["imu"]["is_valid"] << 0) | (r["accelerometer"]["is_valid"] << 1) | (has_temperature << 2)
    sensor_id = r["sensor_id"].encode("utf-8")

    out = b"AIS2"
    out += struct.pack("<BB", 1, flags)                        # version, flags
    out += struct.pack("<Iq", r["sequence"], r["timestamp_ms"])  # u32, i64
    out += struct.pack("<B", len(sensor_id)) + sensor_id
    out += struct.pack("<3d", r["imu"]["roll"], r["imu"]["pitch"], r["imu"]["yaw"])
    out += struct.pack("<3d", *(r["accelerometer"][k] for k in "xyz"))
    if has_temperature:
        out += struct.pack("<d", r["temperature"])
    return out


for name in ("reading", "reading_no_temperature"):
    reading = json.loads((DATA / f"{name}.json").read_text(encoding="utf-8"))
    encoded = encode(reading)
    (DATA / f"{name}.bin").write_bytes(encoded)
    print(f"{name}.bin: {len(encoded)} bytes")
    print("  " + encoded.hex(" "))
