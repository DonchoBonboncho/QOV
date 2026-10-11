#include "../QOV.h"
#include "../cli/image_io.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: qov-encode OUTPUT.qov FRAME1.ppm [FRAME2.ppm ...]\n"
                     "       PNG frames are supported when compiled with OpenCV.\n";
        return 2;
    }
    try {
        if (static_cast<unsigned long long>(argc - 2) > qov::kMaxFrames)
            throw std::runtime_error("Too many frames");
        const auto first = qov::read_image(argv[2]);
        qov::Header header;
        header.width = first.width;
        header.height = first.height;
        header.frames = static_cast<std::uint32_t>(argc - 2);
        std::ofstream out(argv[1], std::ios::binary);
        if (!out) throw std::runtime_error("Cannot create output file");
        qov::write_header(out, header);
        std::vector<std::uint8_t> previous;
        for (int i = 2; i < argc; ++i) {
            qov::Image image = i == 2 ? first : qov::read_image(argv[i]);
            if (image.width != header.width || image.height != header.height)
                throw std::runtime_error("Frame dimensions differ: " + std::string(argv[i]));
            auto compressed = qov::encode_frame(image.rgb, previous.empty() ? nullptr : &previous);
            qov::write_frame(out, compressed);
            std::cout << "Frame " << (i - 1) << '/' << header.frames << ": "
                      << image.rgb.size() << " -> " << compressed.size() << " bytes\n";
            previous = std::move(image.rgb);
        }
        out.close();
        if (!out) throw std::runtime_error("Failed to finish output file");
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "qov-encode: " << e.what() << '\n';
        return 1;
    }
}
