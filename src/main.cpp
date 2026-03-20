#include "editor.hpp"
#include "version.hpp"
#include <fstream>
#include <iostream>
#include <print>

using namespace std::literals;

const std::string raw_logo_path = "logo_raw.txt";
const std::string license_path = "LICENSE";

/// @brief Usage instructions for the command-line interface, called when -h or --help is passed, or when invalid arguments are given
/// @param program_name The name of the program (typically argv[0])
void print_usage(std::string_view program_name)
{
    std::println("Usage: {} [-d] <filename>", program_name);
    std::println("Options:");
    std::println("  -d,--debug    Enable debug mode (show key sequences)");
    std::println("  -v,--version    Show version information");
    std::println("  -l,--license    Show license information");
    std::println("  --splash, --logo    Show splash screen");
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    bool debug_mode = false;
    std::string filename;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-d" || arg == "--debug") {
            debug_mode = true;
        } else if (arg == "-v" || arg == "--version") {
            std::println("{} {}", BZ_NOTA_APP_NAME, BZ_NOTA_VERSION);
            return 0;
        } else if (arg == "--splash" || arg == "--logo") {
            std::ifstream file(raw_logo_path, std::ios::binary);
            if (file) {
                std::cout << file.rdbuf() << "\x1b[0m" << std::endl;
            } else {
                std::println("Logo file not found: {}", raw_logo_path);
            }
            return 1;
        } else if (arg == "-l" || arg == "--license") {
            std::ifstream file(license_path, std::ios::binary);
            if (file) {
                std::ostringstream oss;
                oss << file.rdbuf();
                std::println("\n{}", oss.str());
            } else {
                std::println("License file not found: {}", license_path);
            }
            return 1;
        }  else if (arg.starts_with("-")) {
            print_usage(argv[0]);
            return 1;
        } else {
            filename = std::move(arg); // First non-option is the filename
        }
    }
    
    if (filename.empty()) {
        filename = "Untitled"sv; // Default filename if none provided
    }
    
    try {
        Editor editor(filename, debug_mode);
        editor.run();
    } catch (const std::exception& e) {
        std::println("Error: {}", e.what());
        return 1;
    }
    
    return 0;
}