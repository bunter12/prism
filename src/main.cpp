#include "cli.hpp"
#include "core/mapped_file.hpp"
#include "tui/tui_include.hpp"
#include "formats/detector.hpp"
#include "formats/image.hpp"

#include <iostream>
#include <algorithm>
#include <format>

int main(int argc, char* argv[]) {
    auto options = prism::cli::parse(argc, argv);

    switch (options.action) {
        case prism::cli::Action::PrintHelp:
            prism::cli::print_help(argv[0]);
            return 0;

        case prism::cli::Action::PrintVersion:
            prism::cli::print_version();
            return 0;

        case prism::cli::Action::Error:
            std::cerr << "Error: " << options.error_message << "\n\n";
            prism::cli::print_help(argv[0]);
            return 1;

        case prism::cli::Action::Run:
            try {
                prism::core::MappedFile file(options.file_path);

                if (file.empty()) {
                    std::cout << "[File is completely empty]\n";
                    return 0;
                }
                
                auto file_type = prism::formats::detect_file_type(file.span());
                auto type_name = prism::formats::file_type_to_string(file_type);
                std::string file_display_name = options.file_path.filename().string();

                if (prism::formats::is_image(file_type)) {
                    auto img = prism::formats::parse_image(file.span(), file_type);

                    std::cout << "\n prism Image Inspector\n";
                    std::cout << "  ──────────────────────────────────────────────────\n";
                    std::cout << std::format("  File:         {}\n", file_display_name);
                    std::cout << std::format("  Format:       {}\n", img ? img->format_name : "Unknown Image");
                    std::cout << std::format("  File Size:    {} bytes\n", file.size());

                    if (img) {
                        std::cout << std::format("  Resolution:   {} x {} px\n", img->width, img->height);
                        std::cout << std::format("  Megapixels:   {:.2f} MP\n", img->megapixels());
                        std::cout << std::format("  Color Model:  {}\n", img->color_model);
                        std::cout << std::format("  Bit Depth:    {}-bit per channel\n", img->bit_depth);
                        std::cout << std::format("  Channels:     {}\n", img->channels);
                    } else {
                        std::cout << "  [Failed to parse image header: corrupt or truncated]\n";
                    }

                    std::cout << "  ──────────────────────────────────────────────────\n\n";
                    return 0;
                }

                if (prism::formats::is_archive(file_type)) {
                    std::cout << "📦 Detected " << type_name 
                              << ": " << file_display_name << " (" << file.size() << " bytes)\n"
                              << "Interactive Archive Tree View will be enabled in Phase 5!\n";
                    return 0;
                }

                std::string title = std::format("{} [{}]", file_display_name, type_name);

                prism::tui::RawModeGuard guard;
                prism::tui::TextPager pager(file.span());
                pager.enter_alternate_screen();

                size_t top_line = 0;
                size_t left_col = 0;
                pager.render(top_line, left_col, title);

                while (true) {
                    auto event = prism::tui::read_key();
                    auto term_size = prism::tui::get_terminal_size();
                    size_t content_rows = (term_size.rows > 1) ? term_size.rows - 1 : 1;
                    size_t max_top = pager.max_top_line(content_rows);

                    if (event.key == prism::tui::Key::Resize) {
                        top_line = std::min(top_line, max_top);
                        size_t max_col = pager.max_left_col(term_size.cols);
                        left_col = std::min(left_col, max_col);

                        pager.render(top_line, left_col, title);
                        continue;
                    }
                    
                    if (event.is_char('q') || event.key == prism::tui::Key::Escape) {
                        break;
                    }

                    if (event.is_char('j') || event.key == prism::tui::Key::ArrowDown) {
                        if (top_line < max_top) {
                            ++top_line;
                            pager.render(top_line, left_col, title);
                        }
                    }
                    else if (event.is_char('k') || event.key == prism::tui::Key::ArrowUp) {
                        if (top_line > 0) {
                            --top_line;
                            pager.render(top_line, left_col, title);
                        }
                    }
                    else if (event.is_char('l') || event.key == prism::tui::Key::ArrowRight) {
                        size_t max_col = pager.max_left_col(term_size.cols);
                        if (left_col < max_col) {
                            left_col = std::min(left_col + 8, max_col);
                            pager.render(top_line, left_col, title);
                        }
                    }
                    else if (event.is_char('h') || event.key == prism::tui::Key::ArrowLeft) {
                        left_col = (left_col >= 8) ? left_col - 8 : 0;
                        pager.render(top_line, left_col, title);
                    }
                    else if (event.key == prism::tui::Key::PageDown || event.is_char(' ')) {
                        top_line = std::min(top_line + content_rows, max_top);
                        pager.render(top_line, left_col, title);
                    }
                    else if (event.key == prism::tui::Key::PageUp) {
                        top_line = (top_line > content_rows) ? top_line - content_rows : 0;
                        pager.render(top_line, left_col, title);
                    }
                    else if (event.is_char('g') || event.key == prism::tui::Key::Home) {
                        top_line = 0;
                        left_col = 0;
                        pager.render(top_line, left_col, title);
                    }
                    else if (event.is_char('G') || event.key == prism::tui::Key::End) {
                        top_line = max_top;
                        pager.render(top_line, left_col, title);
                    }
                }

                pager.leave_alternate_screen();

            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
                return 1;
            }
            break;
    }

    return 0;
}