#include "QOV.h"

#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Byte = std::uint8_t;
using Bytes = std::vector<Byte>;

void expect_error(const std::string& label, const std::function<void()>& callback) {
    bool threw = false;
    try { callback(); } catch (const std::exception&) { threw = true; }
    if (!threw) throw std::runtime_error("Should reject " + label);
}

void check_video(const std::vector<Bytes>& frames, std::uint32_t width) {
    assert(!frames.empty());
    const auto height = static_cast<std::uint32_t>(frames[0].size() / 3 / width);
    qov::Header header{width, height, static_cast<std::uint32_t>(frames.size()), 24, 1};
    std::ostringstream encoded_file(std::ios::binary);
    qov::write_header(encoded_file, header);
    Bytes previous;
    for (const auto& frame : frames) {
        const auto compressed = qov::encode_frame(frame, previous.empty() ? nullptr : &previous);
        qov::write_frame(encoded_file, compressed);
        previous = frame;
    }
    std::istringstream source(encoded_file.str(), std::ios::binary);
    auto parsed = qov::read_header(source);
    assert(parsed.width == width && parsed.height == height &&
           parsed.frames == frames.size() && parsed.fps_num == 24 && parsed.fps_den == 1);
    previous.clear();
    for (const auto& expected : frames) {
        const auto payload = qov::read_frame(source, qov::frame_bytes(parsed) / 3);
        const auto result = qov::decode_frame(payload, expected.size() / 3,
                                              previous.empty() ? nullptr : &previous);
        if (expected != result) throw std::runtime_error("Lossless round trip failed");
        previous = result;
    }
    qov::require_eof(source);
}

void run() {
    check_video({Bytes{0, 0, 0}}, 1);
    check_video({Bytes{255, 255, 255}}, 1);
    check_video({Bytes{15, 7, 8}, Bytes{30, 14, 16}}, 1);
    check_video({Bytes(3 * 128, 0), Bytes(3 * 128, 0)}, 16);
    check_video({Bytes(3 * 127, 255), Bytes(3 * 127, 0)}, 127);
    check_video({Bytes{0,0,0, 32,0,0, 0,0,0, 32,0,0, 0,0,0}}, 5); // hash collisions
    check_video({Bytes{1,2,3, 4,5,6, 7,8,9}, Bytes{7,8,9, 1,2,3, 4,5,6}}, 3);
    check_video({Bytes{10,20,30, 20,30,40, 10,20,30, 20,30,40}}, 4);

    std::mt19937 engine(1234567);
    for (int trial = 0; trial < 400; ++trial) {
        const std::size_t width = 1 + engine() % 39;
        const std::size_t height = 1 + engine() % 31;
        const int frame_count = 1 + engine() % 5;
        std::vector<Bytes> frames;
        for (int f = 0; f < frame_count; ++f) {
            Bytes rgb(width * height * 3);
            for (std::size_t i = 0; i < rgb.size(); i += 3) {
                if (f > 0 && engine() % 4 == 0) {
                    rgb[i] = frames[f-1][i];
                    rgb[i+1] = frames[f-1][i+1];
                    rgb[i+2] = frames[f-1][i+2];
                } else if (i != 0 && engine() % 3 == 0) {
                    rgb[i] = rgb[i-3]; rgb[i+1] = rgb[i-2]; rgb[i+2] = rgb[i-1];
                } else {
                    rgb[i] = static_cast<Byte>(engine() % 256);
                    rgb[i+1] = static_cast<Byte>(engine() % 256);
                    rgb[i+2] = static_cast<Byte>(engine() % 256);
                }
            }
            frames.push_back(std::move(rgb));
        }
        check_video(frames, static_cast<std::uint32_t>(width));
    }

    // Verify that the encoder actually uses the inter-frame copy operation.
    const Bytes distinct{25, 80, 100, 55, 120, 200, 200, 11, 1};
    const auto copied = qov::encode_frame(distinct, &distinct);
    assert(copied.size() == 1 && copied[0] == 0x82);

    expect_error("zero width", [] { qov::frame_bytes({0, 1, 1, 30, 1}); });
    expect_error("huge dimensions", [] { qov::frame_bytes({UINT32_MAX, UINT32_MAX, 1, 30, 1}); });
    expect_error("wrong frame dimensions", [] {
        const Bytes other{1,2,3,4,5,6};
        qov::encode_frame(Bytes{1,2,3}, &other);
    });
    expect_error("empty frame", [] { qov::encode_frame(Bytes{}, nullptr); });
    expect_error("zero fps", [] {
        std::ostringstream out; qov::write_header(out, {1, 1, 1, 0, 1});
    });
    expect_error("legacy header", [] {
        std::istringstream in("xxxx\x01\x00\x00\x00", std::ios::binary);
        qov::read_header(in);
    });
    expect_error("short header", [] {
        std::istringstream in("QOV1\x01", std::ios::binary);
        qov::read_header(in);
    });
    expect_error("missing frame", [] {
        std::istringstream in("", std::ios::binary); qov::read_frame(in, 1);
    });
    expect_error("oversized compressed frame", [] {
        const std::string bad{"\x05\x00\x00\x00", 4};
        std::istringstream in(bad, std::ios::binary); qov::read_frame(in, 1);
    });
    expect_error("truncated compressed frame", [] {
        const std::string bad{"\x01\x00\x00\x00", 4};
        std::istringstream in(bad, std::ios::binary); qov::read_frame(in, 1);
    });
    expect_error("truncated diff", [] { qov::decode_frame(Bytes{0x20}, 1, nullptr); });
    expect_error("truncated RGB", [] { qov::decode_frame(Bytes{0x60, 1}, 1, nullptr); });
    expect_error("uninitialized index", [] { qov::decode_frame(Bytes{0x40}, 1, nullptr); });
    expect_error("copy in first frame", [] { qov::decode_frame(Bytes{0x80}, 1, nullptr); });
    expect_error("run past frame", [] { qov::decode_frame(Bytes{0x01}, 1, nullptr); });
    expect_error("invalid opcode", [] { qov::decode_frame(Bytes{0xa0}, 1, nullptr); });
    expect_error("invalid RGB opcode", [] { qov::decode_frame(Bytes{0x61, 1, 2, 3}, 1, nullptr); });
    expect_error("invalid diff channel", [] { qov::decode_frame(Bytes{0x3f, 0x00}, 1, nullptr); });
    expect_error("leftover compressed bytes", [] {
        qov::decode_frame(Bytes{0x00, 0x00}, 1, nullptr);
    });
    expect_error("leftover file data", [] {
        std::istringstream in("x"); qov::require_eof(in);
    });
}
} // namespace

int main() {
    try {
        run();
        std::cout << "QOV1: 400 randomized video tests and malformed input tests passed\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
