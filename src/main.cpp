#include <iostream>
#include "cli.hpp"
#include "core/mapped_file.hpp"
#include "tui/terminal.hpp"
#include "tui/keyboard.hpp"
#include <system_error>

int main(int argc, char* argv[]) {
    prism::cli::Options options = prism::cli::parse(argc, argv);

    switch (options.action) {
        case prism::cli::Action::PrintHelp:
            prism::cli::print_help(argv[0]);
            return 0;
        case prism::cli::Action::PrintVersion:
            prism::cli::print_version();
            return 0;
        case prism::cli::Action::Run:
            try {
                prism::tui::RawModeGuard tui{};
                prism::core::MappedFile file(options.file_path);

                std::cout << "Opened: " << options.file_path << " (" << file.size() << " bytes)\r\n";
                std::cout << "Use [j / Down] to scroll down, [k / Up] to scroll up, [q] to exit.\r\n";

                {
                    prism::tui::RawModeGuard guard;

                    int current_line = 0;

                    while (true) {
                        auto event = prism::tui::read_key();

                        if (event.is_char('q')) {
                            std::cout << "\r\nExiting...\r\n";
                            break;
                        }

                        if (event.key == prism::tui::Key::ArrowDown || event.is_char('j')) {
                            ++current_line;
                            std::cout << "Scroll DOWN -> Line: " << current_line << "\r\n";
                        }
                        else if (event.key == prism::tui::Key::ArrowUp || event.is_char('k')) {
                            if (current_line > 0) --current_line;
                            std::cout << "Scroll UP   -> Line: " << current_line << "\r\n";
                        }
                        else if (event.key == prism::tui::Key::PageDown) {
                            current_line += 10;
                            std::cout << "PAGE DOWN   -> Line: " << current_line << "\r\n";
                        }
                        else if (event.key == prism::tui::Key::PageUp) {
                            current_line = (current_line >= 10) ? current_line - 10 : 0;
                            std::cout << "PAGE UP     -> Line: " << current_line << "\r\n";
                        }
                    }
                }

                std::cout << "Terminal successfully restored.\n";

            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
                return 1;
            }
            break;
        case prism::cli::Action::Error:
            std::cerr << "Error: " << options.error_message << "\n";
            return 1;
    }

    return 0;
}