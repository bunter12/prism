#include <iostream>
#include "cli.hpp"
#include "core/mapped_file.hpp"
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
            try{
                std::cout << "Opening file: " << options.file_path << "\n";

                prism::core::MappedFile file(options.file_path);

                std::cout << "Successfully mapped! Size: " << file.size() << " bytes" << "\n";

                if (file.empty()){
                    std::cout << "File is empty" << "\n";
                    return 0;
                }

                auto bytes = file.span();

                std::cout << "First 16 bytes: ";
                size_t preview_len = std::min(bytes.size(), static_cast<size_t>(16));
                for (size_t i = 0; i < preview_len; ++i) {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]) << " ";
                }
                std::cout << std::dec << "\n";
                break;
            } catch (const std::system_error& e) {
                const auto code = e.code();

                if (code == std::errc::permission_denied) {
                    std::cerr << "Error: Permission denied. You do not have read access to '" 
                              << options.file_path.string() << "'.\n";
                } 
                else if (code == std::errc::no_such_file_or_directory) {
                    std::cerr << "Error: File not found: '" 
                              << options.file_path.string() << "'.\n";
                } 
                else if (code == std::errc::is_a_directory) {
                    std::cerr << "Error: '" << options.file_path.string() 
                              << "' is a directory, not a regular file.\n";
                } 
                else {
                    std::cerr << "System Error: " << e.what() << "\n";
                }
                return 1;
            } catch (const std::exception& e) {
                std::cerr << "Fatal Error: " << e.what() << "\n";
                return 1;
            }
        case prism::cli::Action::Error:
            std::cerr << "Error: " << options.error_message << "\n";
            return 1;
    }

    return 0;
}