#include <editor.hpp>
#include <version.hpp>
#include <fstream>
#include <iostream>
#include <print>
#include <filesystem>

using namespace std::literals;

namespace fs = std::filesystem;

fs::path get_data_dir() {
    if (const char* env = std::getenv("BZNOTA_DATA_DIR")) {
        return fs::path(env);
    }
    
    fs::path exe_path = fs::read_symlink("/proc/self/exe").parent_path();
    
    fs::path local = exe_path / "share/bznota";
    if (fs::exists(local)) {
        return local;
    }
    
    return fs::path("/usr/local/share/bznota");
}

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
            fs::path logo_path = get_data_dir() / "logo_raw.txt";
            std::ifstream file(logo_path, std::ios::binary);
            if (file) {
                std::cout << file.rdbuf() << "\x1b[0m" << std::endl;
            } else {
                std::println("Logo file not found: {}", logo_path.string());
            }
            return 1;
        } else if (arg == "-l" || arg == "--license") {
            fs::path license_path = get_data_dir() / "LICENSE";
            std::ifstream file(license_path, std::ios::binary);
            if (file) {
                std::ostringstream oss;
                oss << file.rdbuf();
                std::println("\n{}", oss.str());
            } else {
                std::println("License file not found: {}", license_path.string());
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
