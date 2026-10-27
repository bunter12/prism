#pragma once
#include <filesystem>
#include <string_view>

namespace prism::cli {

enum class Action {
    PrintHelp,
    PrintVersion,
    Run,
    Error
};

struct Options {
    Action action{Action::Run};
    std::filesystem::path file_path{};
    bool hex_mode{false};
    bool in_place{false};
    std::string error_message{};
};

Options parse(int argc, char* argv[]);

void print_help(std::string_view program_name);
void print_version();

} // namespace prism::cli