#ifndef QOV_IMAGE_IO_H
#define QOV_IMAGE_IO_H

#include <cstdint>
#include <string>
#include <vector>

namespace qov {
struct Image {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<std::uint8_t> rgb;
};

Image read_image(const std::string& filename);
void write_ppm(const std::string& filename, const Image& image);
} // namespace qov
#endif
