#include "editor.hpp"
#include "version.hpp"
#include <iostream>
#include <fstream>
#include <string>

const std::string raw_logo_path = "logo_raw.txt";
const std::string license_path = "LICENSE";

/// @brief Usage instructions for the command-line interface, called when -h or --help is passed, or when invalid arguments are given
/// @param program_name The name of the program (typically argv[0])
void print_usage(const std::string& program_name)
{
    std::cout << "Usage: " << program_name << " [-d] <filename>\n";
    std::cout << "Options:\n";
    std::cout << "  -d,--debug    Enable debug mode (show key sequences)\n";
    std::cout << "  -v,--version    Show version information\n";
    std::cout << "  -l,--license    Show license information\n";
    std::cout << "  --splash, --logo    Show splash screen\n";
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
            std::cout << BZ_NOTA_APP_NAME << " " << BZ_NOTA_VERSION << "\n";
            return 0;
        } else if (arg == "--splash" || arg == "--logo") {
            std::ifstream file(raw_logo_path, std::ios::binary);
            if (file) {
                // Read and print directly to preserve every escape code and Unicode character
                std::cout << file.rdbuf() << "\x1b[0m" << std::endl;
            } else {
                std::cerr << "Logo file not found: " << raw_logo_path << std::endl;
            }
            return 1;
        } else if (arg == "-l" || arg == "--license") {
            std::ifstream file(license_path, std::ios::binary);
            if (file) {
                std::cout << "\n" << std::endl;
                std::cout << file.rdbuf() << std::endl;
            } else {
                std::cerr << "License file not found: " << license_path << std::endl;
            }
            return 1;
        }  else if (arg[0] == '-') {
            print_usage(argv[0]);
            return 1;
        } else {
            filename = arg;  // First non-option argument is the filename
        }
    }
    
    if (filename.empty()) {
        filename = "Untitled"; // Default filename if none provided
    }
    
    try {
        Editor editor(filename, debug_mode);
        editor.run(); // Launch the ftxui-based editor interface
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}