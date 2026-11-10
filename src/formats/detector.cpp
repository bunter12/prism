#include "formats/detector.hpp"
#include <cstring>

namespace prism::formats {

namespace {

bool starts_with_bytes(std::span<const uint8_t> data, std::span<const uint8_t> magic) noexcept {
    if (data.size() < magic.size()) return false;
    return std::memcmp(data.data(), magic.data(), magic.size()) == 0;
}

bool is_plain_text(std::span<const uint8_t> data) noexcept {
    size_t check_len = std::min<size_t>(data.size(), 512);
    for (size_t i = 0; i < check_len; ++i) {
        if (data[i] == 0x00) {
            return false;
        }
    }
    return true; 
}

bool is_tar_archive(std::span<const uint8_t> data) noexcept {
    if (data.size() < 512) return false;
    const uint8_t* ustar = data.data() + 257;
    return std::memcmp(ustar, "ustar", 5) == 0;
}

bool is_macho_binary(std::span<const uint8_t> data) noexcept {
    if (data.size() < 4) return false;
    uint32_t magic = (static_cast<uint32_t>(data[0]) << 24) |
                     (static_cast<uint32_t>(data[1]) << 16) |
                     (static_cast<uint32_t>(data[2]) << 8)  |
                      static_cast<uint32_t>(data[3]);

    return magic == 0xFEEDFACF || magic == 0xCFFAEDFE || // Mach-O 64-bit
           magic == 0xFEEDFACE || magic == 0xCEFAEDFE || // Mach-O 32-bit
           magic == 0xCAFEBABE || magic == 0xBEBAFECA;   // Universal / Fat binary
}

} // namespace


FileType detect_file_type(std::span<const uint8_t> data) noexcept {
    if (data.empty()) {
        return FileType::PlainText;
    }

    static constexpr uint8_t png_magic[] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    if (starts_with_bytes(data, png_magic)) {
        return FileType::Png;
    }

    static constexpr uint8_t jpeg_magic[] = {0xFF, 0xD8, 0xFF};
    if (starts_with_bytes(data, jpeg_magic)) {
        return FileType::Jpeg;
    }

    if (data.size() >= 6 && (std::memcmp(data.data(), "GIF87a", 6) == 0 || 
                             std::memcmp(data.data(), "GIF89a", 6) == 0)) {
        return FileType::Gif;
    }

    static constexpr uint8_t gzip_magic[] = {0x1F, 0x8B};
    if (starts_with_bytes(data, gzip_magic)) {
        return FileType::Gzip;
    }

    static constexpr uint8_t zip_magic[] = {'P', 'K', 0x03, 0x04};
    static constexpr uint8_t zip_empty[] = {'P', 'K', 0x05, 0x06};
    if (starts_with_bytes(data, zip_magic) || starts_with_bytes(data, zip_empty)) {
        return FileType::Zip;
    }

    if (is_tar_archive(data)) {
        return FileType::Tar;
    }

    static constexpr uint8_t elf_magic[] = {0x7F, 'E', 'L', 'F'};
    if (starts_with_bytes(data, elf_magic)) {
        return FileType::Elf;
    }

    if (is_macho_binary(data)) {
        return FileType::MachO;
    }

    if (is_plain_text(data)) {
        return FileType::PlainText;
    }

    return FileType::UnknownBinary;
}

std::string_view file_type_to_string(FileType type) noexcept {
    switch (type) {
        case FileType::PlainText:     return "Text";
        case FileType::Png:           return "PNG Image";
        case FileType::Jpeg:          return "JPEG Image";
        case FileType::Gif:           return "GIF Image";
        case FileType::Zip:           return "ZIP Archive";
        case FileType::Tar:           return "TAR Archive";
        case FileType::Gzip:          return "GZIP Stream";
        case FileType::Elf:           return "ELF Binary";
        case FileType::MachO:         return "Mach-O Binary";
        case FileType::UnknownBinary: return "Binary";
    }
}

bool is_image(FileType type) noexcept {
    return type == FileType::Png || type == FileType::Jpeg || type == FileType::Gif;
}

bool is_archive(FileType type) noexcept {
    return type == FileType::Zip || type == FileType::Tar || type == FileType::Gzip;
}

} // namespace prism::formats