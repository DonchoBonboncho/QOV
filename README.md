# QOV — lossless RGB video codec (QOV1 prototype)

QOV is an experimental, simple **lossless video** codec inspired by the simplicity
of QOI (Quite OK Image). It encodes ordered frames without modifying their
**8-bit RGB channel values**, and reuses unchanged areas of the preceding frame.

**Scope:** QOV1 stores only 8-bit, three-channel RGB video (no audio, alpha,
16-bit/HDR data, color metadata, or timing changes). Input images must all have
the same width and height. Frame-rate metadata is currently fixed at 30/1 fps
by the CLI. QOV is a prototype, not an established interoperable video format.

**Compatibility warning:** QOV1 is a new, versioned binary format. It is **not**
compatible with `.dat` streams produced by the original unversioned prototype.
The old encoder/decoder were not reliable under hash collisions. Keep a copy of
the old revision if you need to inspect legacy data.

## Build

Requires a C++17 compiler and CMake 3.16+. OpenCV (core + imgcodecs) is
**optional**: with OpenCV installed, the encoder also reads PNG and other
supported 8-bit 3-channel images. Without it, input must be binary P6 PPM.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Encode and decode

For lossless fidelity, use PPM or 8-bit, RGB PNG source frames (not JPG).
PNG files with alpha or grayscale are rejected rather than silently discarding
channels. The encoder expects paths in display order, e.g. zero-padded names:

```sh
./build/qov-encode video.qov frames/0001.png frames/0002.png frames/0003.png
./build/qov-decode video.qov restored_frames
```

Or use the PPM-only mode without OpenCV:

```sh
./build/qov-encode video.qov frames/0001.ppm frames/0002.ppm
./build/qov-decode video.qov restored_frames
```

The decoder writes `restored_frames/frame000001.ppm`, etc., with exact RGB
bytes. To build a playable video, a separate player or muxer can use the
framerate metadata. The codec does not store audio.

## QOV1 wire format

All integers are unsigned 32-bit **little-endian**. Pixels are stored in
**RGB** order, row-major, with three 8-bit channels.

| Bytes | Field |
|---|---|
| 4 | ASCII magic `QOV1` |
| 4 | Width (>0) |
| 4 | Height (>0) |
| 4 | Number of frames (>0) |
| 4 | Frame rate numerator (>0) |
| 4 | Frame rate denominator (>0) |
| Repeated per frame | 4-byte compressed payload size followed by that many bytes |

Each frame starts with an empty 32-entry pixel cache and `prev = (0,0,0)`.
For frames after the first, the decoder also has the completely reconstructed
previous frame. The stream represents exactly `width × height` pixels per frame.

| Leading 3 bits | Remaining bits | Decoding operation |
|---|---|---|
| `000` | `nnnnn` | RUN: output `n+1` copies of `prev` |
| `001` | `rrrrr`, then `ggggbbbb` | DIFF: `r=prev.r-(rrrrr-15)`, `g=prev.g-(gggg-7)`, `b=prev.b-(bbbb-7)` |
| `010` | `iiiii` | INDEX: output cache slot `i` (must be initialized) |
| `011` | `00000`, then `R G B` | RGB: output 3 channel bytes |
| `100` | `nnnnn` | COPY: output `n+1` consecutive pixels at the same positions from the preceding frame |
| `101`–`111` | — | Reserved; reject |

Every output pixel—including those emitted by RUN, COPY, INDEX, and DIFF—sets
`prev` and updates the cache slot:

```cpp
index = (r * 3 + g * 5 + b * 7) & 31;
cache[index] = {r,g,b};
```

Pixel-cache replacement is deterministic and mirrored by the encoder and
decoder. Runs may cross scanlines but cannot cross frame boundaries; COPY is
invalid in frame zero. All payloads must be consumed **exactly**, with no
trailing bytes after the last frame.

The decoder rejects invalid dimensions, impossible sizes, truncated streams,
unknown opcodes, invalid cache references, out-of-range RGB differences,
run overflow and extra data. It caps each frame at 64 Mi pixels, and the
container at one million frames. These are implementation limits, not
recommendations for routine use.

## Testing

`tests/test_codec.cpp` exercises deterministic and 400 randomized videos,
cache collisions, video-copy operations, and malformed streams. `tests/test_cli.py`
checks end-to-end file-to-file round trips and rejects truncated or trailing
files. Both are run by CTest and GitHub Actions using GCC and Clang.

For truly lossless validation, compare the decoded **raw RGB bytes** with the
RGB bytes read by the encoder, not PNG file sizes (PNG metadata and packing
can differ while pixels are identical).

## Future directions

- Streaming APIs for files with unknown frame counts, rather than fixed headers.
- Periodic intra frames to support seeking and recovery from corruption.
- RGBA/16-bit formats with explicit channel and color-space metadata.
- Compression benchmarks against lossless FFV1 and x264 lossless presets.
- Better temporal prediction than same-position COPY, with controlled complexity.

