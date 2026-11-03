#pragma once
#include <cstdint>

namespace prism::tui {

enum class Key : int {
    None = 0,
    Char,
    
    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,

    PageUp,
    PageDown,
    Home,
    End,

    Enter,
    Backspace,
    Escape
};

struct KeyEvent {
    Key key{Key::None};
    char ch{'\0'};

    [[nodiscard]] bool is_char(char expected) const noexcept {
        return key == Key::Char && ch == expected;
    }
};

KeyEvent read_key();
} // namespace prism::tui