#pragma once
#include <termios.h>
#include <unistd.h>

namespace prism::tui{
class RawModeGuard{
public:
    RawModeGuard();
    ~RawModeGuard();

    RawModeGuard(const RawModeGuard&) = delete;
    RawModeGuard& operator=(const RawModeGuard&) = delete;

    RawModeGuard(RawModeGuard&&) noexcept = default;
    RawModeGuard& operator=(RawModeGuard&&) noexcept = default;

    void set_echo(bool enable);
    void set_signals(bool enable);
private:
    void apply_changes();

    struct termios orig_termios_{};
    struct termios curr_termios_{};
};
} // namespace prism::tui