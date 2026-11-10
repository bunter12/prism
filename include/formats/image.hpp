#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include "formats/detector.hpp"

namespace prism::formats{

struct ImageInfo{
    uint32_t width{0};
    uint32_t height{0};
    uint8_t bit_depth{0};
    uint8_t channels{0};
    std::string format_name{};
    std::string color_model{};

    [[nodiscard]] double megapixels() const noexcept {
        return (static_cast<double>(width) * height) / 1'000'000.0;
    }
};

std::optional<ImageInfo> parse_png(std::span<const uint8_t> data) noexcept;
std::optional<ImageInfo> parse_jpeg(std::span<const uint8_t> data) noexcept;

std::optional<ImageInfo> parse_image(std::span<const uint8_t> data, FileType type) noexcept;
} // namespace prism::formats