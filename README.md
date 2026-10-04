# AIS2203 — Compulsory assignment 1: Telemetry on the wire

A small sensor node produces one kind of value, a `SensorData`: an IMU, an
accelerometer, an optional temperature, a sensor id, a sequence number and a
timestamp. In this assignment you take that one value to bytes and back
**three ways** — JSON, YAML, and a binary format of your own that a Python
script can read.

It goes with the **Serialization** lecture (week 39) and the
[Serialization](https://markaren.github.io/E-book_cps/Chapter4/serialization/)
page of the book.

| Part | You build | File you edit | Rough time |
|------|-----------|---------------|------------|
| 0 | Set-up: open, build, push, see CI run | — | 30 min |
| 1 | Read a file; parse JSON and YAML; bad input → `ParseError` | `src/text_formats.cpp` | 2 h |
| 2 | Write JSON and YAML; exact round trips | `src/text_formats.cpp` | 1–2 h |
| 3 | A binary format from a byte-layout table, checked against files written by Python | `src/binary_format.cpp` | 2–3 h |
| — | `ANSWERS.md` | `ANSWERS.md` | 1 h |

Do the parts in order. Part 3 is where the time goes.

## What "approved" means

1. All **three CI jobs are green** in your own GitHub repository: GCC, MSVC
   and the address sanitizer.
2. **`ANSWERS.md` is filled in** — short answers, your own words, your own
   measurements.
3. You have **not edited** anything in `include/`, `tests/`, `data/`,
   `CMakeLists.txt` or `.github/`. Everything you write goes in `src/`.

**Hand in on Canvas:**

- a **screenshot** of your repository's **Actions** page showing one run with
  **all three jobs green** — open the run, so the job names and the commit are
  visible;
- your filled-in **`ANSWERS.md`**.

Your repository stays private; nobody needs access to it. The deadline is on
**Canvas**.

This is individual work. Discuss ideas with each other as much as you like;
write your own code. If you use an AI assistant, use it the way the
[book suggests](https://markaren.github.io/E-book_cps/using_ai/): to explain,
not to write the thing you are here to learn.

---

## Part 0 — Set-up

1. Extract the zip, **File → Open** the folder in CLion. The first CMake
   configure downloads and builds three libraries (nlohmann/json, yaml-cpp,
   Catch2) — a few minutes, internet required.
2. Build all, then run the tests: the **All CTest** run configuration, or
   `ctest --test-dir cmake-build-debug --output-on-failure`. **Everything is
   red.** That is the starting point.
3. Create a **private** GitHub repository, commit everything, push. Open the
   **Actions** tab and watch the three jobs go red there too.
4. If you did the debugging lab you already have a WSL toolchain. Add a CMake
   profile on it with `-DLAB_SANITIZER=address,undefined` now; you will want
   it in Part 3.

Dependencies come through CMake's `FetchContent`, exactly as the labs fetch
Catch2. (vcpkg is week 41 — after that you would use it instead.)

Every push runs all three jobs, and GitHub counts the minutes against your
account's free quota. Push when a part is done, not after every line.

## Part 1 — Text in

`data/reading.json` and `data/reading.yaml` hold the same reading. Implement
`readFile`, `parseJson` and `parseYaml`.

- Every field is **required**, except `temperature`, which may be absent.
- **Deserializing is parsing input you do not control.** The `data/broken_*`
  files are truncated, have a field missing, or have a string where a number
  belongs. Every one of them must produce an `ais2203::ParseError` — not a
  `json::parse_error`, not a `YAML::BadConversion`, and above all not a
  `SensorData` full of zeros that looks like a real reading.
- The libraries stay **out of the header**. `text_formats.hpp` does not
  include them and must not need to.

Hints. nlohmann/json: `json::parse`, `at("key")` (throws if missing — unlike
`operator[]`), `get_to`, `contains`; all its exceptions derive from
`json::exception`. Writing `to_json`/`from_json` for the three structs, as on
the lecture slide, makes Part 2 nearly free. yaml-cpp: `YAML::Load`,
`node["key"]`, `as<double>()`, and `if (node["key"])` to test for presence; all
its exceptions derive from `YAML::Exception`. Look at the `"y"` key in
`reading.yaml` and read the comment next to it.

## Part 2 — Text out

Implement `toJson` and `toYaml`, so that `parseJson(toJson(x)) == x` — for
**every** `x`.

- Doubles must survive **bit for bit**. `0.1 + 0.2` is `0.30000000000000004`;
  a writer that prints six significant digits sends `0.3`, and the test is red.
  Find out how many digits are enough, or let the library worry about it.
- An empty `temperature` is **left out** — not written as `0`, not as `null`.
- The tests use sensor ids with quotes, backslashes, `: `, `#` and `å` in
  them. Build the output with the library — a `json` object and `dump()`, or
  `YAML::Emitter` — which quotes and escapes for you. Concatenating strings by
  hand breaks on these ids.

## Part 3 — Binary, by hand

"Don't roll your own" is the rule, and you will follow it in the project. You
break it once here, because writing a byte layout yourself is how padding and
endianness stop being abstract — and because device manuals and protocol
specifications describe their data exactly like this, as a table of bytes.

The layout is the table at the top of `include/ais2203/binary_format.hpp`.
`tools/make_golden.py` is the same layout in twelve lines of Python; it wrote
`data/reading.bin`. **Your `encodeBinary` must produce the same 87 bytes, and
your `decodeBinary` must read them** — two programs in two languages, agreeing
on nothing but a table.

- Write **values, not memory**: shifts, one byte at a time, as on the lecture
  slide. No `memcpy` of the struct, no `#pragma pack`, no `reinterpret_cast`
  of a pointer to a number. For a `double`, `std::bit_cast<std::uint64_t>(d)`
  (C++20, `<bit>`) gives you its 64 bits as an integer.
- The decoder's input is **whatever arrived**. The tests feed it all 87
  truncations of a valid message, and a message whose `sensor_id` length byte
  claims 255. Each must be a `ParseError` — and the decoder must never read
  outside its input. MSVC will not notice if it does. The **address sanitizer**
  job will. Run the test under your ASan profile before you push.
- A hex view helps: `xxd data/reading.bin` in WSL, or CLion's *View as →
  Hex*. Compare with your own output byte by byte; the first difference tells
  you which field is wrong.

## The demo, and `ANSWERS.md`

When all three parts are green, run the `sizes` target. It prints how many
bytes the same reading takes in each format. Put the table in `ANSWERS.md` and
answer the questions there — a few sentences each. `ANSWERS.md` is the only
place you write answers.
