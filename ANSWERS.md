# Compulsory assignment 1 — answers

Name:

A few sentences per question, in your own words. "I measured … and saw …"
beats a textbook paragraph every time.

## 1. Sizes

Paste the output of the `sizes` program:

```
/mnt/d/Skole/semester5/kyberfysisk/obligAssignment1/cmake-build-debug-asan/sizes
One SensorData (sensor_id "imu-north-01", with temperature):

  toJson(), as you format it             214 bytes     100 %
  toJson(), whitespace removed           214 bytes     100 %
  toYaml()                               206 bytes      96 %
  encodeBinary()                          87 bytes      41 %

At 50 Hz: 10700 B/s as compact JSON, 4350 B/s as binary.

Process finished with exit code 0

```

a) Binary is smaller than compact JSON — but nowhere near ten times smaller.
Where do the bytes of the binary message go? (Count them per field.)
Answer:
The magic takes 4 bytes, version 1, flags 1, sequence 4, timestamp 8, N 1, the sensor ID 12, IMU is 3 doubles so 24, accelerometer is also 24, and temperature is 8. This gives 87 bytes in total. The seven doubles make up 56/87 bytes. Each double always takes 8 bytes in binary, even when the value is something simple like 0 or 21.5.
binary only saves the names and punctuation, while the values themselves don't shrink much. The doubles take 8 bytes each, which is often more than JSON's text, and the sensor id is the same 12 bytes in both formats.

b) Roughly what fraction of the compact JSON is field *names* and punctuation
rather than values? What kind of format would avoid sending those at all, and
what does it require both ends to have?
Answer:
The actual values in the JSON take about 64 characters, so around 214 - 64 = 150 characters are field names and punctuation. That is roughly 70% of the compact JSON. A schema-based binary format such as Protocol Buffers could avoid sending the field names in every message. Both sides would need to have the same schema beforehand so they know what each field means.

c) At 50 Hz, would you switch this message from JSON to binary on a Wi-Fi
link? On a 9600-baud serial line (≈ 960 bytes/s)? Show the arithmetic.
Answer: On Wi-Fi I would keep JSON. At 50 Hz it uses 10,700 B/s × 8 = 85,600 bit/s, or about 86 kbit/s, which is very small compared with a normal Wi-Fi connection.

On a 9600-baud serial connection, there are only about 960 B/s available. JSON needs 10,700 B/s, so it needs about 11 times more bandwidth than is available. Binary needs 4,350 B/s, which is still about 4.5 times too much. Therefore I would switch to binary, but I would also have to reduce the reading rate or send less data.

## 2. Why not the struct?

Give three **separate** reasons why

```cpp
file.write(reinterpret_cast<const char*>(&reading), sizeof(reading));
```

does not produce a file another program can read — for *this* `SensorData`.
For one of them, say what `sizeof(ais2203::SensorData)` is on your machine and
how many of those bytes are actual data.

1. std::string is not its characters.** Its layout is implementation-specific: a pointer, a size, and possibly a small inline buffer. For a short ID like "imu-north-01" the characters may sit inside the object, but for a longer ID they live on the heap and the file only receives a memory address from my process. Either way, another program has no way to recover the text.

2. Padding. The compiler inserts invisible bytes so that each member is correctly aligned, for example after sequence (4 bytes) so that timestamp_ms (8 bytes) starts on an 8-byte boundary, and after the bool flags before the next double. These bytes are uninitialised, so the file would also contain whatever happened to be on the stack.
   On my machine sizeof(ais2203::SensorData) is 128 bytes, but only 83 of them are actual data (sequence 4, timestamp 8, sensor ID 12, seven doubles 56, three presence flags 3). The remaining 45 bytes are padding and std::string's internal bookkeeping. std::optional<double> contains no pointer: it stores the double inline plus a bool, followed by padding.

3. Byte order and type sizes are not fixed. The raw bytes are written in my CPU's byte order (little-endian on x86). A reader that assumes big-endian would read a uint32_t sequence of 42 (`2A 00 00 00`) as 704 643 072, with no error. The sizes of types like long and of library types like std::string also differ between compilers and platforms. A Python script reading the file would have to guess all of this, while a defined format writes each value byte by byte in an agreed order.

## 3. Untrusted input

a) Disable your decoder's bounds check for a moment (comment it out) and run
`test_03_binary` twice: in your normal build, and in your ASan profile. What
does each one report? Why does the normal build not notice that the decoder
reads outside its input? Put the check back afterwards.

Answer:
With the need() checks commented out, the normal build (MinGW GCC 15.2, Debug) aborts with exit code 3 on the first test, "a sensor_id length that lies is a ParseError". The message comes from a libstdc++ assertion in std::span::operator[] (span:302) saying the index is out of range. Because the whole process dies, the other 8 tests never run and are all reported as failed.

The ASan build stops on the same test and reports a heap-buffer-overflow, READ of size 1, in Reader::u8 (binary_format.cpp:66), called from decodeBinary at binary_format.cpp:182. That is the loop that reads the sensor_id characters. The length byte claims more characters than the message contains, so the loop reads past the end. ASan also shows that the buffer was allocated by the test's readBytes (test_03_binary.cpp:109), which confirms that the read went one byte past the end of that vector.

A normal build usually does not notice, because without the check bytes[pos] is just a read from memory. The bytes right after the buffer still belong to the process, so the read succeeds and returns whatever happens to be there. Nothing crashes, and the decoder quietly returns garbage. That is undefined behaviour, and nothing in a plain build looks for it. ASan marks the memory around every allocation as forbidden and stops the program on the first access to it. My normal build caught it only because GCC 15 turns on _GLIBCXX_ASSERTIONS by default at -O0, which bounds-checks std::span::operator[]. In a Release build, or with an older compiler, that check is gone and the bad read goes unnoticed. I put the checks back afterwards, and all tests pass.

b) `parseJson` must throw `ParseError` rather than let `json::type_error`
escape. What does a caller gain from that? Connect it to what is — and is not
— included in `text_formats.hpp`.

Answer:
The caller only has to know about one error type, ParseError, which is declared in our own header. text_formats.hpp does not include nlohmann/json, so a caller cannot even name json::type_error or json::parse_error without pulling in the JSON library themselves. If those exceptions escaped, every caller would have to depend on a library that is supposed to be an internal detail. With ParseError, one catch block handles all bad input, whether it came from JSON, YAML or binary, and the message describes our problem ("missing field value") instead of the library's. It also means we could replace nlohmann/json with another library later without changing or recompiling any code that calls parseJson. This is the lecture's rule of catching errors at the boundary and translating them into your own type.
## 4. Looking ahead: a stream of readings

Next week these bytes go over a TCP connection, which delivers a stream of
bytes with no message boundaries. Why could you not simply put a `'\n'` after
each binary reading to mark where it ends? Point at a specific byte in
`data/reading.bin` that proves it, and say which field it belongs to.

Answer:
A '\n' is just the byte 0x0A, and in a binary message that value can appear anywhere inside the data itself. Integers and doubles are written as raw bytes, so any of their bytes can happen to be 0x0A. A reader splitting on '\n' would see that byte as the end of the message and cut it in two.

data/reading.bin proves it: the byte at offset 6 is 0x0A. Offset 6 is the first byte of sequence, which is written little-endian, so this is its lowest byte (sequence = 10). A reader splitting on '\n' would end the message after only six bytes, right after the magic, version and flags. The byte at offset 73 is also 0x0A, inside the double accelerometer.z.

A delimiter only works when it cannot occur inside a message, which holds for compact JSON text (no raw newlines) but not for binary. For binary data, the fix is a length prefix: send the size of the message first, then exactly that many bytes. The reader then knows where the message ends without looking at its contents.
