#include "QOV.h"

#include <array>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>

namespace qov {
namespace {

using Pixel = std::array<std::uint8_t, 3>;

struct Cache {
    std::array<Pixel, 32> pixels{};
    std::array<bool, 32> valid{};

    static std::size_t slot(const Pixel& p) {
        return (p[0] * 3u + p[1] * 5u + p[2] * 7u) & 31u;
    }
    void update(const Pixel& p) {
        const auto index = slot(p);
        pixels[index] = p;
        valid[index] = true;
    }
    bool contains(const Pixel& p) const {
        const auto index = slot(p);
        return valid[index] && pixels[index] == p;
    }
};

Pixel pixel_at(const std::vector<std::uint8_t>& bytes, std::size_t index) {
    const auto offset = index * 3;
    return {bytes[offset], bytes[offset + 1], bytes[offset + 2]};
}

void append_pixel(std::vector<std::uint8_t>& out, const Pixel& p,
                  Cache& cache, Pixel& previous) {
    out.insert(out.end(), p.begin(), p.end());
    cache.update(p);
    previous = p;
}

void write_u32(std::ostream& out, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        out.put(static_cast<char>((value >> (i * 8)) & 255u));
    }
    if (!out) throw std::runtime_error("Failed to write QOV stream");
}

std::uint32_t read_u32(std::istream& in) {
    std::uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        const int byte = in.get();
        if (byte == std::char_traits<char>::eof())
            throw std::runtime_error("Truncated QOV header or frame length");
        value |= static_cast<std::uint32_t>(byte) << (i * 8);
    }
    return value;
}

std::size_t validate_dimensions(std::uint32_t width, std::uint32_t height) {
    if (width == 0 || height == 0 ||
        static_cast<std::uint64_t>(width) * height > kMaxPixels)
        throw std::runtime_error("Invalid or oversized QOV dimensions");
    return static_cast<std::size_t>(width) * height;
}

void validate_pixels(const std::vector<std::uint8_t>& pixels,
                     const std::vector<std::uint8_t>* previous) {
    if (pixels.empty() || pixels.size() % 3 != 0 ||
        pixels.size() / 3 > kMaxPixels)
        throw std::runtime_error("Invalid RGB frame size");
    if (previous && previous->size() != pixels.size())
        throw std::runtime_error("Previous frame has wrong dimensions");
}

} // namespace

std::size_t frame_bytes(const Header& header) {
    return validate_dimensions(header.width, header.height) * 3;
}

void write_header(std::ostream& output, const Header& header) {
    frame_bytes(header);
    if (header.frames == 0 || header.frames > kMaxFrames ||
        header.fps_num == 0 || header.fps_den == 0)
        throw std::runtime_error("Invalid QOV frame count or frame rate");
    output.write("QOV1", 4);
    write_u32(output, header.width);
    write_u32(output, header.height);
    write_u32(output, header.frames);
    write_u32(output, header.fps_num);
    write_u32(output, header.fps_den);
}

Header read_header(std::istream& input) {
    char magic[4] = {};
    if (!input.read(magic, 4) || std::string(magic, 4) != "QOV1")
        throw std::runtime_error("Not a QOV1 stream (legacy files are unsupported)");
    Header header;
    header.width = read_u32(input);
    header.height = read_u32(input);
    header.frames = read_u32(input);
    header.fps_num = read_u32(input);
    header.fps_den = read_u32(input);
    frame_bytes(header);
    if (header.frames == 0 || header.frames > kMaxFrames ||
        header.fps_num == 0 || header.fps_den == 0)
        throw std::runtime_error("Invalid QOV frame count or frame rate");
    return header;
}

std::vector<std::uint8_t> encode_frame(
    const std::vector<std::uint8_t>& rgb,
    const std::vector<std::uint8_t>* previous_frame) {
    validate_pixels(rgb, previous_frame);
    const auto count = rgb.size() / 3;
    Cache cache;
    Pixel previous{0, 0, 0};
    std::vector<std::uint8_t> encoded;
    encoded.reserve(rgb.size());

    for (std::size_t i = 0; i < count;) {
        const Pixel current = pixel_at(rgb, i);
        std::size_t run = 0;
        while (run < 32 && i + run < count &&
               pixel_at(rgb, i + run) == previous)
            ++run;

        std::size_t copy = 0;
        if (previous_frame) {
            while (copy < 32 && i + copy < count &&
                   pixel_at(rgb, i + copy) == pixel_at(*previous_frame, i + copy))
                ++copy;
        }

        // 000nnnnn: repeat the immediately preceding pixel n+1 times.
        // 100nnnnn: take n+1 pixels from the same positions in the prior frame.
        if (run != 0 && run >= copy) {
            encoded.push_back(static_cast<std::uint8_t>(run - 1));
            for (std::size_t j = 0; j < run; ++j) cache.update(previous);
            i += run;
            continue;
        }
        if (copy != 0) {
            encoded.push_back(static_cast<std::uint8_t>(0x80u | (copy - 1)));
            for (std::size_t j = 0; j < copy; ++j) {
                previous = pixel_at(*previous_frame, i + j);
                cache.update(previous);
            }
            i += copy;
            continue;
        }

        if (cache.contains(current)) {
            encoded.push_back(static_cast<std::uint8_t>(0x40u | Cache::slot(current)));
        } else {
            // Delta from the previous output pixel, not previous-frame pixel.
            const int dr = static_cast<int>(previous[0]) - current[0];
            const int dg = static_cast<int>(previous[1]) - current[1];
            const int db = static_cast<int>(previous[2]) - current[2];
            if (dr >= -15 && dr <= 16 && dg >= -7 && dg <= 8 &&
                db >= -7 && db <= 8) {
                encoded.push_back(static_cast<std::uint8_t>(0x20u | (dr + 15)));
                encoded.push_back(static_cast<std::uint8_t>(((dg + 7) << 4) | (db + 7)));
            } else {
                encoded.push_back(0x60u);
                encoded.insert(encoded.end(), current.begin(), current.end());
            }
        }
        cache.update(current);
        previous = current;
        ++i;
    }
    return encoded;
}

std::vector<std::uint8_t> decode_frame(
    const std::vector<std::uint8_t>& encoded, std::size_t pixel_count,
    const std::vector<std::uint8_t>* previous_frame) {
    if (pixel_count == 0 || pixel_count > kMaxPixels ||
        (previous_frame && previous_frame->size() != pixel_count * 3))
        throw std::runtime_error("Invalid decoded frame dimensions");
    Cache cache;
    Pixel previous{0, 0, 0};
    std::vector<std::uint8_t> decoded;
    decoded.reserve(pixel_count * 3);
    std::size_t position = 0;
    std::size_t cursor = 0;
    while (position < pixel_count) {
        if (cursor == encoded.size())
            throw std::runtime_error("Truncated QOV frame opcode");
        const std::uint8_t command = encoded[cursor++];
        const std::uint8_t type = command >> 5;
        const std::size_t parameter = command & 31u;
        if (type == 0 || type == 4) {
            const std::size_t length = parameter + 1;
            if (length > pixel_count - position)
                throw std::runtime_error("QOV run exceeds frame boundary");
            if (type == 4 && !previous_frame)
                throw std::runtime_error("Previous-frame opcode in first frame");
            for (std::size_t j = 0; j < length; ++j) {
                const Pixel p = type == 0 ? previous
                    : pixel_at(*previous_frame, position + j);
                append_pixel(decoded, p, cache, previous);
            }
            position += length;
        } else if (type == 1) {
            if (cursor == encoded.size())
                throw std::runtime_error("Truncated QOV diff opcode");
            const auto second = encoded[cursor++];
            const int r = static_cast<int>(previous[0]) - (static_cast<int>(parameter) - 15);
            const int g = static_cast<int>(previous[1]) - (static_cast<int>(second >> 4) - 7);
            const int b = static_cast<int>(previous[2]) - (static_cast<int>(second & 15) - 7);
            if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
                throw std::runtime_error("QOV diff produces invalid RGB channel");
            append_pixel(decoded, {static_cast<std::uint8_t>(r),
                                   static_cast<std::uint8_t>(g),
                                   static_cast<std::uint8_t>(b)}, cache, previous);
            ++position;
        } else if (type == 2) {
            if (!cache.valid[parameter])
                throw std::runtime_error("QOV index refers to uninitialized pixel");
            append_pixel(decoded, cache.pixels[parameter], cache, previous);
            ++position;
        } else if (type == 3) {
            if (parameter != 0 || encoded.size() - cursor < 3)
                throw std::runtime_error("Invalid or truncated QOV RGB opcode");
            const Pixel rgb{encoded[cursor], encoded[cursor + 1], encoded[cursor + 2]};
            cursor += 3;
            append_pixel(decoded, rgb, cache, previous);
            ++position;
        } else {
            throw std::runtime_error("Unknown QOV opcode");
        }
    }
    if (cursor != encoded.size())
        throw std::runtime_error("Extra encoded data after QOV frame");
    return decoded;
}

void write_frame(std::ostream& output, const std::vector<std::uint8_t>& encoded) {
    if (encoded.empty() || encoded.size() > std::numeric_limits<std::uint32_t>::max())
        throw std::runtime_error("Invalid encoded frame size");
    write_u32(output, static_cast<std::uint32_t>(encoded.size()));
    output.write(reinterpret_cast<const char*>(encoded.data()),
                 static_cast<std::streamsize>(encoded.size()));
    if (!output) throw std::runtime_error("Failed writing encoded frame");
}

std::vector<std::uint8_t> read_frame(std::istream& input, std::size_t pixel_count) {
    if (pixel_count == 0 || pixel_count > kMaxPixels)
        throw std::runtime_error("Invalid frame size");
    const auto size = read_u32(input);
    // At most four bytes per output RGB pixel (raw opcode + 3 channels).
    if (size == 0 || size > pixel_count * 4)
        throw std::runtime_error("Invalid compressed frame length");
    std::vector<std::uint8_t> encoded(size);
    if (!input.read(reinterpret_cast<char*>(encoded.data()),
                    static_cast<std::streamsize>(encoded.size())))
        throw std::runtime_error("Truncated compressed frame payload");
    return encoded;
}

void require_eof(std::istream& input) {
    if (input.peek() != std::char_traits<char>::eof())
        throw std::runtime_error("Trailing bytes after QOV video");
    if (input.bad()) throw std::runtime_error("Error reading QOV stream");
}

} // namespace qov
