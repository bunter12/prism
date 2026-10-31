#pragma once 
#include <filesystem>
#include <span>

namespace prism::core{
class MappedFile{
public:
    MappedFile() noexcept = default;

    explicit MappedFile(const std::filesystem::path& path);

    ~MappedFile();

    MappedFile(const MappedFile&) = delete;
    MappedFile& operator=(const MappedFile&) = delete;

    MappedFile(MappedFile&& other) noexcept;
    MappedFile& operator=(MappedFile&& other) noexcept;

    [[nodiscard]] std::span<const uint8_t> span() const noexcept;
    [[nodiscard]] const uint8_t* data() noexcept;
    [[nodiscard]] size_t size() const noexcept;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] bool is_mapped() const noexcept;

private:
    void cleanup() noexcept;

    const uint8_t* data_{nullptr};
    size_t size_{0};
};
} // namespace prism::core