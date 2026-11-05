#include "tui/terminal_guard.hpp"
#include <csignal>
#include <signal.h>
#include <system_error>
#include <unistd.h>
#include <cstdlib>
namespace prism::tui{

namespace {

volatile std::sig_atomic_t g_window_resized = 0;

void winresize_handler(int) {
    g_window_resized = 1;
}

} // namespace

RawModeGuard::RawModeGuard() {
    if (!isatty(STDIN_FILENO)) {
        throw std::system_error(errno, std::generic_category(), "Standard input is not a TTY");
    }

    if (tcgetattr(STDIN_FILENO, &orig_termios_) == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to get terminal attributes");
    }

    curr_termios_ = orig_termios_;

    curr_termios_.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    curr_termios_.c_lflag &= ~(ICANON | ECHO | IEXTEN);
    curr_termios_.c_lflag |= ISIG;

    curr_termios_.c_cc[VMIN] = 1;
    curr_termios_.c_cc[VTIME] = 0;

    apply_changes();

    setup_signals();
}

RawModeGuard::~RawModeGuard() {
    restore_terminal();
    active_instance_ = nullptr;
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

void RawModeGuard::set_signals(bool enable) {
    if (enable) {
        curr_termios_.c_lflag |= ISIG;
    } else {
        curr_termios_.c_lflag &= ~ISIG;
    }
    apply_changes();
}

void RawModeGuard::setup_signals() {
    active_instance_ = this;
    std::atexit(restore_terminal);

    struct sigaction sa{};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    ::sigaction(SIGINT,  &sa, nullptr);
    ::sigaction(SIGTERM, &sa, nullptr);
    ::sigaction(SIGQUIT, &sa, nullptr);
    ::sigaction(SIGHUP,  &sa, nullptr);

    struct sigaction sa_win{};
    sa_win.sa_handler = winresize_handler;
    sigemptyset(&sa_win.sa_mask);
    sa_win.sa_flags = 0;
    ::sigaction(SIGWINCH, &sa_win, nullptr);
}

void RawModeGuard::restore_terminal() noexcept {
    const char cleanup_cmd[] = "\033[?25h\033[?1049l";
    ::write(STDOUT_FILENO, cleanup_cmd, sizeof(cleanup_cmd) - 1);

    if (active_instance_ != nullptr) {
        ::tcsetattr(STDIN_FILENO, TCSAFLUSH, &active_instance_->orig_termios_);
    }
}

void RawModeGuard::signal_handler(int signum) {
    restore_terminal();
    ::signal(signum, SIG_DFL);
    ::raise(signum);
}

bool RawModeGuard::check_and_reset_resize() noexcept {
    if (g_window_resized != 0) {
        g_window_resized = 0;
        return true;
    }
    return false;
}

} // namespace prism::tui