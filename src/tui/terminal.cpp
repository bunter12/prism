#include "tui/terminal.hpp"
#include <system_error>
namespace prism::tui{

RawModeGuard::RawModeGuard(){
    if (!isatty(STDIN_FILENO)) {
        throw std::system_error(errno, std::generic_category(), "Standart input is not a TTY");
    }

    if (tcgetattr(STDIN_FILENO, &orig_termios_) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to get terminal attibutes");
    }

    curr_termios_ = orig_termios_;

    curr_termios_.c_lflag &= ~(ICANON | ECHO | IEXTEN);
    curr_termios_.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    // curr_termios.c_oflag &= ~(OPOST);

    curr_termios_.c_cc[VMIN] = 1;
    curr_termios_.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &curr_termios_) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to set terminal to raw mode");
    }
}

RawModeGuard::~RawModeGuard(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios_);
}

void RawModeGuard::set_echo(bool enable) {
    if (enable) {
        curr_termios_.c_lflag |= ECHO;
    } else {
        curr_termios_.c_lflag &= ~ECHO;
    }
    apply_changes();
}

void RawModeGuard::apply_changes() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &curr_termios_);
}

} // namespace prism::tui