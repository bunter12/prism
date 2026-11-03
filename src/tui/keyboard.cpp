#include "tui/keyboard.hpp"
#include <poll.h>
#include <unistd.h>

namespace prism::tui {

namespace{

bool has_pending_input(int timeout_ms) {
    struct pollfd pfd{};
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    return ::poll(&pfd, 1, timeout_ms) > 0;
} 

} //namespace

KeyEvent read_key() {
    char c{'\0'};
    
    if (::read(STDIN_FILENO, &c, 1) <= 0) {
        return KeyEvent{Key::None, '\0'};
    }

    if (c == '\x1b') {
        if (!has_pending_input(50)) {
            return KeyEvent{Key::Escape, '\0'};
        }

        char seq[3];
        if (::read(STDIN_FILENO, &seq[0], 1) <= 0) return KeyEvent{Key::Escape, '\0'};

        if (seq[0] == '['){
            if (!has_pending_input(50)) return KeyEvent{Key::Escape, '\0'};
            if (::read(STDIN_FILENO, &seq[1], 1) <= 0) return KeyEvent{Key::Escape, '\0'};

            switch (seq[1]) {
                case 'A': return KeyEvent{Key::ArrowUp, '\0'};
                case 'B': return KeyEvent{Key::ArrowDown, '\0'};
                case 'C': return KeyEvent{Key::ArrowRight, '\0'};
                case 'D': return KeyEvent{Key::ArrowLeft, '\0'};
                case 'H': return KeyEvent{Key::Home, '\0'};
                case 'F': return KeyEvent{Key::End, '\0'};
            }

            if (seq[1] >= '0' && seq[1] <= '9'){
                if (!has_pending_input(50)) return KeyEvent{Key::Escape, '\0'};
                if (::read(STDIN_FILENO, &seq[2], 1) <= 0) return KeyEvent{Key::Escape, '\0'};

                if (seq[2] == '~'){
                    switch (seq[1]) {
                        case '1': return KeyEvent{Key::Home, '\0'};
                        case '3': return KeyEvent{Key::Backspace, '\0'};
                        case '4': return KeyEvent{Key::End, '\0'};
                        case '5': return KeyEvent{Key::PageUp, '\0'};
                        case '6': return KeyEvent{Key::PageDown, '\0'};
                        case '7': return KeyEvent{Key::Home, '\0'};
                        case '8': return KeyEvent{Key::End, '\0'};
                    }
                }
            }
        }
        else if (seq[0] == 'O') {
            if (!has_pending_input(50)) return {Key::Escape, '\0'};
            if (::read(STDIN_FILENO, &seq[1], 1) <= 0) return {Key::Escape, '\0'};
            if (seq[1] == 'H') return {Key::Home, '\0'};
            if (seq[1] == 'F') return {Key::End, '\0'};
        }

        return KeyEvent{Key::Escape, '\0'};
    }

    switch (c) {
        case '\r':
        case '\n':
            return KeyEvent{Key::Enter, '\0'};
        case '\b':
            return {Key::Backspace, '\0'};
        case 127:
            return KeyEvent{Key::Backspace, '\0'};
        default:
            return KeyEvent{Key::Char, c};
    }
}
} // namespace prism::tui