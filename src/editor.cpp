#include "editor.hpp"
#include <fstream>
#include <sstream>

Editor::Editor(const std::string& filename) : filename(filename) {
    load_file();
}

void Editor::load_file() {
    buffer.clear();
    std::ifstream file(filename);
    if (!file) {
        // If file doesn't exist, start with an empty buffer
        buffer.push_back("");
        modified = false;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        buffer.push_back(line);
    }
    if (buffer.empty()) buffer.push_back("");
    modified = false;
}

void Editor::save_file() {
    std::ofstream file(filename);
    for (size_t i = 0; i < buffer.size(); ++i) {
        file << buffer[i];
        if (i + 1 < buffer.size()) file << '\n';
    }
    modified = false;
}

const std::vector<std::string>& Editor::get_buffer() const {
    return buffer;
}

bool Editor::is_modified() const {
    return modified;
}
