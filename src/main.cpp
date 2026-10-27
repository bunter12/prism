#include <iostream>
#include "cli.hpp"

int main(int argc, char* argv[]) {
    prism::cli::Options options = prism::cli::parse(argc, argv);

    switch (options.action) {
        case prism::cli::Action::PrintHelp:
            prism::cli::print_help(argv[0]);
            break;
        case prism::cli::Action::PrintVersion:
            prism::cli::print_version();
            break;
        case prism::cli::Action::Run:
            std::cout << "Target file: " << options.file_path << "\n";
            std::cout << "Hex mode:    " << (options.hex_mode ? "ON" : "OFF") << "\n";
            std::cout << "In-place:    " << (options.in_place ? "ON" : "OFF") << "\n";
            break;
        case prism::cli::Action::Error:
            std::cerr << "Error: " << options.error_message << "\n";
            return 1;
    }

    return 0;
}