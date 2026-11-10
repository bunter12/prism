#pragma once
#include <span>
#include <cstdint>
#include <string_view>

namespace prism::formats{

enum class FileType{
    Png,
    Jpeg,
    Tar,
    Zip,
    Gzip,
    Gif,
    Elf,
    MachO,
    PlainText,
    UnknownBinary
};

FileType detect_file_type(std::span<const uint8_t> data) noexcept;

std::string_view file_type_to_string(FileType type) noexcept;

bool is_image(FileType type) noexcept;

bool is_archive(FileType type) noexcept;

} // namespace prism::formats