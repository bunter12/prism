#include "cli.hpp"
#include <span>
#include <iostream>


namespace prism::cli {
    Options parse(int argc, char* argv[]){
        Options options;

        if (argc < 2) {
            options.action = Action::Error;
            options.error_message = "No input file provided. Use --help for usage details.";
            return options;
        }
        
        std::span args(argv + 1, static_cast<size_t>(argc - 1));

        for (size_t i = 0; i < args.size(); ++i) {
            std::string_view arg = args[i];

            if (arg == "--help" || arg == "-h") {
                options.action = Action::PrintHelp;
                return options;
            } else if (arg == "--version" || arg == "-v") {
                options.action = Action::PrintVersion;
                return options;
            } else if (arg == "--hex") {
                options.hex_mode = true;
            } else if (arg == "--in-place") {
                options.in_place = true;
            } else if (arg[0] == '-') {
                options.action = Action::Error;
                options.error_message = "Unknown option: " + std::string(arg);
                return options;
            } else {
                if (!options.file_path.empty()) {
                options.action = Action::Error;
                options.error_message = "Multiple target files provided. Only one file is supported.";
                return options;
                }

                options.file_path = arg;
            }
        } 

        if (options.file_path.empty()) {
            options.action = Action::Error;
            options.error_message = "Target file path is missing.";
        }

        return options;
    }


    void print_help(std::string_view program_name) {
        std::cout << "Usage: " << program_name << " [options] <file_path>\n"
                  << "Options:\n"
                  << "  --help, -h       Show this help message\n"
                  << "  --version, -v    Show version information\n"
                  << "  --hex            Enable hex mode\n"
                  << "  --in-place       Modify the file in place\n";
    }

    void print_version() {
        std::cout << "prism version 0.1.0\n" << "Built with Modern C++20 (macOS/Linux POSIX)\n";
    }
} // namespace prism::cli