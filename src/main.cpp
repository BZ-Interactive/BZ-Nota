#include "editor.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [-d] <filename>\n";
        std::cerr << "Options:\n";
        std::cerr << "  -d    Enable debug mode (show key sequences)\n";
        return 1;
    }
    
    // Parse command-line arguments
    bool debug_mode = false;
    std::string filename;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-d" || arg == "--debug") {
            debug_mode = true;
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            std::cerr << "Usage: " << argv[0] << " [-d] <filename>\n";
            return 1;
        } else {
            filename = arg;  // First non-option argument is the filename
        }
    }
    
    if (filename.empty()) {
        std::cerr << "Error: No filename specified\n";
        std::cerr << "Usage: " << argv[0] << " [-d] <filename>\n";
        return 1;
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
