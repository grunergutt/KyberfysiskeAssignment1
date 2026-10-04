# Compulsory assignment 1 — answers

Name:

A few sentences per question, in your own words. "I measured … and saw …"
beats a textbook paragraph every time.

## 1. Sizes

Paste the output of the `sizes` program:

```
(paste here)
```

a) Binary is smaller than compact JSON — but nowhere near ten times smaller.
Where do the bytes of the binary message go? (Count them per field.)

b) Roughly what fraction of the compact JSON is field *names* and punctuation
rather than values? What kind of format would avoid sending those at all, and
what does it require both ends to have?

c) At 50 Hz, would you switch this message from JSON to binary on a Wi-Fi
link? On a 9600-baud serial line (≈ 960 bytes/s)? Show the arithmetic.

## 2. Why not the struct?

Give three **separate** reasons why

```cpp
file.write(reinterpret_cast<const char*>(&reading), sizeof(reading));
```

does not produce a file another program can read — for *this* `SensorData`.
For one of them, say what `sizeof(ais2203::SensorData)` is on your machine and
how many of those bytes are actual data.

## 3. Untrusted input

a) Disable your decoder's bounds check for a moment (comment it out) and run
`test_03_binary` twice: in your normal build, and in your ASan profile. What
does each one report? Why does the normal build not notice that the decoder
reads outside its input? Put the check back afterwards.

b) `parseJson` must throw `ParseError` rather than let `json::type_error`
escape. What does a caller gain from that? Connect it to what is — and is not
— included in `text_formats.hpp`.

## 4. Looking ahead: a stream of readings

Next week these bytes go over a TCP connection, which delivers a stream of
bytes with no message boundaries. Why could you not simply put a `'\n'` after
each binary reading to mark where it ends? Point at a specific byte in
`data/reading.bin` that proves it, and say which field it belongs to.
