#include "../QOV.h"
#include "../cli/image_io.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: qov-decode INPUT.qov OUTPUT_DIRECTORY\n";
        return 2;
    }
    try {
        std::ifstream in(argv[1], std::ios::binary);
        if (!in) throw std::runtime_error("Cannot open compressed input");
        const auto header = qov::read_header(in);
        const std::size_t pixels = qov::frame_bytes(header) / 3;
        std::filesystem::create_directories(argv[2]);
        std::vector<std::uint8_t> previous;
        for (std::uint32_t i = 0; i < header.frames; ++i) {
            const auto compressed = qov::read_frame(in, pixels);
            auto raw = qov::decode_frame(compressed, pixels, previous.empty() ? nullptr : &previous);
            std::ostringstream name;
            name << "frame" << std::setw(6) << std::setfill('0') << (i + 1) << ".ppm";
            const auto filename = (std::filesystem::path(argv[2]) / name.str()).string();
            qov::write_ppm(filename, {header.width, header.height, raw});
            previous = std::move(raw);
        }
        qov::require_eof(in);
        std::cout << "Decoded " << header.frames << " RGB frames at " << header.width
                  << 'x' << header.height << ", fps=" << header.fps_num << '/'
                  << header.fps_den << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "qov-decode: " << e.what() << '\n';
        return 1;
    }
}
