#ifndef QOV_H
#define QOV_H

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <vector>

// QOV1 stores lossless, 8-bit RGB video frames. This API accepts pixels in
// row-major RGB order. The wire format is documented in README.md.
namespace qov {

struct Header {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t frames = 0;
    std::uint32_t fps_num = 30;
    std::uint32_t fps_den = 1;
};

// Protect decoders from unreasonable allocations, including malformed inputs.
constexpr std::size_t kMaxPixels = 64u * 1024u * 1024u;
constexpr std::uint32_t kMaxFrames = 1'000'000u;

std::size_t frame_bytes(const Header& header);
void write_header(std::ostream& output, const Header& header);
Header read_header(std::istream& input);

// previous_frame is null on frame zero; otherwise it must contain exactly one
// decoded frame. Each frame has an independently reset pixel-index cache.
std::vector<std::uint8_t> encode_frame(
    const std::vector<std::uint8_t>& rgb,
    const std::vector<std::uint8_t>* previous_frame);

std::vector<std::uint8_t> decode_frame(
    const std::vector<std::uint8_t>& encoded, std::size_t pixel_count,
    const std::vector<std::uint8_t>* previous_frame);

// Frame sizes are little-endian uint32 followed by exactly that many bytes.
void write_frame(std::ostream& output, const std::vector<std::uint8_t>& encoded);
std::vector<std::uint8_t> read_frame(std::istream& input, std::size_t pixel_count);
void require_eof(std::istream& input);

} // namespace qov
#endif
