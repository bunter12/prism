#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace prism::tui {

struct TerminalSize {
    size_t rows{24};
    size_t cols{80};
};

TerminalSize get_terminal_size() noexcept;

class TextPager{
public:
    explicit TextPager(const std::span<const uint8_t> data);
    
    void enter_alternate_screen();
    void leave_alternate_screen();

    void render(size_t top_line, std::string_view file_name = "");

    [[nodiscard]] size_t total_lines() const noexcept { return line_offsets_.size(); }
    [[nodiscard]] size_t max_top_line(size_t screen_rows) const noexcept;
private:
    void build_line_index();
    std::string_view get_line(size_t line_number) const noexcept;

    std::span<const uint8_t> data_;
    std::vector<size_t> line_offsets_;
};

} // namespace prism::tui