#include "tui/renderer.hpp"
#include <sys/ioctl.h>
#include <unistd.h>
#include <string>
#include <format>
namespace prism::tui {

namespace {

std::string format_size(size_t bytes) {
    constexpr double kib = 1024.0;
    constexpr double mib = 1024.0 * 1024.0;
    constexpr double gib = 1024.0 * 1024.0 * 1024.0;

    if (bytes < 1024) {
        return std::format("{} B", bytes);
    } else if (bytes < 1024 * 1024) {
        return std::format("{:.1f} KB", static_cast<double>(bytes) / kib);
    } else if (bytes < 1024 * 1024 * 1024) {
        return std::format("{:.1f} MB", static_cast<double>(bytes) / mib);
    } else {
        return std::format("{:.2f} GB", static_cast<double>(bytes) / gib);
    }
}
} // namespace

TerminalSize get_terminal_size() noexcept {
    struct winsize ws{};
    if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0 && ws.ws_col > 0) {
        return {static_cast<size_t>(ws.ws_row), static_cast<size_t>(ws.ws_col)};
    }
    return {24, 80};
}

TextPager::TextPager(const std::span<const uint8_t> data) : data_(data) {
    build_line_index();
}

void TextPager::build_line_index() {
    line_offsets_.clear();
    max_line_len_ = 0;
    if (data_.empty()) return;

    line_offsets_.push_back(0); 

    size_t line_start = 0;
    for (size_t i = 0; i < data_.size(); ++i) {
        if (data_[i] == '\n') {
            size_t len = (i > line_start && data_[i - 1] == '\r') ? (i - 1 - line_start) : (i - line_start);
            max_line_len_ = std::max(max_line_len_, len);

            if ((i + 1) < data_.size()) {
                line_offsets_.push_back(i + 1);
                line_start = i + 1;
            }
        }
    }
    if (line_start < data_.size()) {
        max_line_len_ = std::max(max_line_len_, data_.size() - line_start);
    }
}

std::string_view TextPager::get_line(size_t line_index) const noexcept {
    if (line_index >= line_offsets_.size()) return {};

    size_t start = line_offsets_[line_index];
    size_t end = (line_index + 1 < line_offsets_.size()) ? line_offsets_[line_index + 1] : data_.size();

    while (end > start && (data_[end - 1] == '\n' || data_[end - 1] == '\r')) {
        --end;
    }

    const char* str_ptr = reinterpret_cast<const char*>(data_.data() + start);
    return {str_ptr, end - start};
}

size_t TextPager::max_top_line([[maybe_unused]] size_t screen_rows) const noexcept {
    if (line_offsets_.empty()) {
        return 0;
    }
    return line_offsets_.size() - 1;
}

size_t TextPager::max_left_col(size_t screen_cols) const noexcept {
    if (max_line_len_ <= screen_cols) {
        return 0;
    }
    return max_line_len_ - screen_cols + 2;
}

void TextPager::enter_alternate_screen() {
    const std::string_view cmd = "\033[?1049h\033[?25l";
    ::write(STDOUT_FILENO, cmd.data(), cmd.size());
}

void TextPager::leave_alternate_screen() {
    const std::string_view cmd = "\033[?25h\033[?1049l";
    ::write(STDOUT_FILENO, cmd.data(), cmd.size());
}

void TextPager::render(size_t top_line, size_t left_col, std::string_view file_name) {
    auto [rows, cols] = get_terminal_size();
    if (rows == 0 || cols == 0) return;

    size_t content_rows = (rows > 1) ? rows - 1 : 1;

    std::string frame;
    frame.reserve(rows * cols + 512);

    for (size_t i = 0; i < content_rows; ++i) {
        frame += std::format("\033[{};1H", i + 1);

        size_t current_line_idx = top_line + i;
        if (current_line_idx < line_offsets_.size()) {
            std::string_view line = get_line(current_line_idx);
            if (left_col < line.size()) {
                line = line.substr(left_col);
                if (line.size() > cols) {
                    line = line.substr(0, cols);
                }
                frame.append(line);
            }
        } else {
            frame += "~";
        }

        frame += "\033[K";
    }
    if (rows > 1) {
        frame += std::format("\033[{};1H", rows);

        size_t total = line_offsets_.size();
        size_t bottom_line = std::min(top_line + content_rows, total);
        size_t percent = (total > 0) ? (bottom_line * 100) / total : 100;
        bool is_end = (bottom_line >= total && top_line > 0);

        std::string name = file_name.empty() ? "prism" : std::string(file_name);
        std::string col_info = (left_col > 0) ? std::format(" [C:{}]", left_col + 1) : "";
        std::string end_tag = is_end ? " [END]" : "";

        std::string status;

        if (cols >= 75) {
            status = std::format(
                " {} ({}) | Lines {}-{}/{}{} ({}%){} | Press 'q' to exit",
                name, format_size(data_.size()), top_line + 1, bottom_line, total, col_info, percent, end_tag
            );
        } else if (cols >= 45) {
            status = std::format(
                " {} ({}) | L {}-{}/{}{} ({}%){}",
                name, format_size(data_.size()), top_line + 1, bottom_line, total, col_info, percent, end_tag
            );
        } else {
            status = std::format(" {} | {}%{}", name, percent, end_tag);
        }

        if (status.size() >= cols) {
            status = status.substr(0, cols > 0 ? cols - 1 : 0);
        }

        frame += "\033[7m";
        frame.append(status);
        frame += "\033[K";
        frame += "\033[0m";
    }

    ::write(STDOUT_FILENO, frame.data(), frame.size());
}
} // namespace prism::tui