#include "image_io.h"
#include "../QOV.h"

#include <cctype>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

#ifdef QOV_HAVE_OPENCV
#include <opencv2/imgcodecs.hpp>
#endif

namespace qov {
namespace {
std::string next_token(std::istream& in) {
    std::string token;
    char ch = 0;
    while (in.get(ch)) {
        if (ch == '#') {
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            token += ch;
            break;
        }
    }
    if (token.empty()) throw std::runtime_error("Incomplete PPM header");
    while (in.get(ch)) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            // The payload begins immediately after this one whitespace delimiter.
            // Handle the conventional CRLF header delimiter as a pair.
            if (ch == '\r' && in.peek() == '\n') in.get();
            break;
        }
        if (ch == '#') {
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        token += ch;
        if (token.size() > 32) throw std::runtime_error("Invalid PPM header token");
    }
    return token;
}

std::uint32_t parse_positive(const std::string& s) {
    if (s.empty() || s[0] == '-') throw std::runtime_error("Invalid PPM dimensions");
    std::size_t used = 0;
    const auto n = std::stoull(s, &used);
    if (used != s.size() || n == 0 || n > UINT32_MAX)
        throw std::runtime_error("Invalid PPM dimension");
    return static_cast<std::uint32_t>(n);
}

Image read_ppm(std::istream& file) {
    if (next_token(file) != "P6")
        throw std::runtime_error("Only binary P6 PPM files are supported without OpenCV");
    Image image;
    image.width = parse_positive(next_token(file));
    image.height = parse_positive(next_token(file));
    if (next_token(file) != "255")
        throw std::runtime_error("Only 8-bit PPM RGB (maxval 255) is supported");
    Header header;
    header.width = image.width;
    header.height = image.height;
    image.rgb.resize(frame_bytes(header));
    if (!file.read(reinterpret_cast<char*>(image.rgb.data()),
                   static_cast<std::streamsize>(image.rgb.size())))
        throw std::runtime_error("Truncated PPM pixel data");
    if (file.peek() != std::char_traits<char>::eof())
        throw std::runtime_error("Unexpected data after PPM pixels");
    return image;
}
} // namespace

Image read_image(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open input image: " + filename);
    char first_two[2] = {};
    if (!in.read(first_two, 2)) throw std::runtime_error("Empty input image: " + filename);
    in.clear();
    in.seekg(0);
    if (first_two[0] == 'P' && first_two[1] == '6') return read_ppm(in);
#ifdef QOV_HAVE_OPENCV
    const cv::Mat frame = cv::imread(filename, cv::IMREAD_UNCHANGED);
    if (frame.empty() || frame.type() != CV_8UC3)
        throw std::runtime_error("Image must be 8-bit, 3-channel RGB (no alpha): " + filename);
    Image image;
    image.width = static_cast<std::uint32_t>(frame.cols);
    image.height = static_cast<std::uint32_t>(frame.rows);
    Header header;
    header.width = image.width;
    header.height = image.height;
    image.rgb.resize(frame_bytes(header));
    for (int y = 0; y < frame.rows; ++y) {
        const auto* row = frame.ptr<cv::Vec3b>(y);
        for (int x = 0; x < frame.cols; ++x) {
            const auto offset = (static_cast<std::size_t>(y) * frame.cols + x) * 3;
            image.rgb[offset]     = row[x][2]; // BGR -> RGB
            image.rgb[offset + 1] = row[x][1];
            image.rgb[offset + 2] = row[x][0];
        }
    }
    return image;
#else
    throw std::runtime_error("PNG/JPEG input requires building with OpenCV: " + filename);
#endif
}

void write_ppm(const std::string& filename, const Image& image) {
    Header header;
    header.width = image.width;
    header.height = image.height;
    if (image.rgb.size() != frame_bytes(header))
        throw std::runtime_error("Incorrect pixel count for PPM output");
    std::ofstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot create output image: " + filename);
    file << "P6\n" << image.width << ' ' << image.height << "\n255\n";
    file.write(reinterpret_cast<const char*>(image.rgb.data()),
               static_cast<std::streamsize>(image.rgb.size()));
    if (!file) throw std::runtime_error("Cannot write image: " + filename);
}
} // namespace qov
