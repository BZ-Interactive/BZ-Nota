#include "editor.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }
    Editor editor(argv[1]);
    // For now, just print the file contents
    const auto& buffer = editor.get_buffer();
    for (const auto& line : buffer) {
        std::cout << line << std::endl;
    }
    // TODO: Add editing and saving logic
    return 0;
}
