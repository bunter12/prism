#include "formats/image.hpp"
#include <optional>
#include <format>

namespace prism::formats {

namespace {

uint16_t read_be16(const uint8_t* ptr) noexcept {
    return static_cast<uint16_t>((static_cast<uint16_t>(ptr[0]) << 8) | 
                                  static_cast<uint16_t>(ptr[1]));
}

uint32_t read_be32(const uint8_t* ptr) noexcept {
    return (static_cast<uint32_t>(ptr[0]) << 24) |
           (static_cast<uint32_t>(ptr[1]) << 16) |
           (static_cast<uint32_t>(ptr[2]) << 8)  |
            static_cast<uint32_t>(ptr[3]);
}

} // namespace

std::optional<ImageInfo> parse_png(std::span<const uint8_t> data) noexcept {
    if (data.size() < 29) {
        return std::nullopt;
    }

    if (std::memcmp(data.data() + 12, "IHDR", 4) != 0) {
        return std::nullopt;
    }
    
    ImageInfo info;
    info.format_name = "PNG image";
    info.width = read_be32(data.data() + 16);
    info.height = read_be32(data.data() + 20);
    info.bit_depth = data[24];

    uint8_t color_type = data[25];
    switch (color_type) {
        case 0: // Grayscale
            info.channels = 1;
            info.color_model = "Grayscale";
            break;
        case 2: // Truecolor
            info.channels = 3;
            info.color_model = "RGB Truecolor";
            break;
        case 3: // Indexed-color
            info.channels = 1;
            info.color_model = "Indexed Color";
            break;
        case 4: // Grayscale with alpha
            info.channels = 2;
            info.color_model = "Grayscale with Alpha";
            break;
        case 6: // Truecolor with alpha
            info.channels = 4;
            info.color_model = "RGBA (Truecolor with Alpha)";
            break;
        default:
            info.color_model = "Unknown Color Model";
            info.channels = 0;
            break;
    }
    return info;
}

std::optional<ImageInfo> parse_jpeg(std::span<const uint8_t> data) noexcept {
    if (data.size() < 4 || data[0] != 0xFF || data[1] != 0xD8) {
        return std::nullopt;
    }
    
    size_t offset = 2;

    while (offset + 4 <= data.size()){
        if (data[offset] != 0xFF) {
            ++offset;
            continue;
        }

        while (offset < data.size() && data[offset] == 0xFF) {
            ++offset;
        }

        if (offset >= data.size()) break;

        uint8_t marker = data[offset++];

        if (marker >= 0xD0 && marker <= 0xD7) continue;
        if (marker == 0xD8) continue;
        if (marker == 0xD9) break;

        if (offset + 2 > data.size()) break;
        uint16_t lenght = read_be16(data.data() + offset);
        
        bool is_sof = (marker >= 0xC0 && marker <= 0xCF) &&
                      (marker != 0xC4 && marker != 0xC8 && marker != 0xCC);
        
        if (is_sof) {
            if (offset + 8 > data.size()) return std::nullopt;

            ImageInfo info;
            info.format_name = (marker == 0xC2) ? "JPEG (Progressive)" : "JPEG (Baseline)";
            info.height = read_be16(data.data() + offset + 3);
            info.width = read_be16(data.data() + offset + 5);
            info.bit_depth = data[offset + 2];
            info.channels = data[offset + 7];

            if (info.channels == 1) info.color_model = "Grayscale";
            else if (info.channels == 3) info.color_model = "YCbCr / sRGB";
            else if (info.channels == 4) info.color_model = "CMYK";
            else info.color_model = std::format("{} channels", info.channels);

            return info;
        }

        offset += lenght;
    }

    return std::nullopt;
}

std::optional<ImageInfo> parse_image(std::span<const uint8_t> data, FileType type) noexcept {
    switch (type) {
        case FileType::Png:
            return parse_png(data);
        case FileType::Jpeg:
            return parse_jpeg(data);
        default:
            return std::nullopt;
    }
}
} // namespace prism::formats