#include "text_buffer.hpp"
#include <fstream>

TextBuffer::TextBuffer() {
    lines.push_back(""); // Start with one empty line
}

bool TextBuffer::load_from_file(const std::string& filename) {
    lines.clear();
    std::ifstream file(filename);
    
    if (!file) {
        // File doesn't exist, start with empty buffer
        lines.push_back("");
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    if (lines.empty()) {
        lines.push_back("");
    }
    
    return true;
}

bool TextBuffer::save_to_file(const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        return false;
    }
    
    for (size_t i = 0; i < lines.size(); ++i) {
        file << lines[i];
        if (i < lines.size() - 1) {
            file << '\n';
        }
    }
    
    return true;
}

const std::string& TextBuffer::get_line(size_t line_index) const {
    return lines[line_index];
}

std::string& TextBuffer::get_line_mut(size_t line_index) {
    return lines[line_index];
}

size_t TextBuffer::line_count() const {
    return lines.size();
}

void TextBuffer::insert_char(size_t line, size_t col, char c) {
    lines[line].insert(col, 1, c);
}

void TextBuffer::delete_char(size_t line, size_t col) {
    if (col < lines[line].length()) {
        lines[line].erase(col, 1);
    }
}

void TextBuffer::insert_line(size_t line_index, const std::string& content) {
    lines.insert(lines.begin() + line_index, content);
}

void TextBuffer::delete_line(size_t line_index) {
    if (lines.size() > 1) {
        lines.erase(lines.begin() + line_index);
    }
}

void TextBuffer::split_line(size_t line, size_t col) {
    std::string& current_line = lines[line];
    std::string right_part = current_line.substr(col);
    current_line = current_line.substr(0, col);
    lines.insert(lines.begin() + line + 1, right_part);
}

void TextBuffer::join_lines(size_t line) {
    if (line + 1 < lines.size()) {
        lines[line] += lines[line + 1];
        lines.erase(lines.begin() + line + 1);
    }
}

bool TextBuffer::is_empty() const {
    return lines.empty() || (lines.size() == 1 && lines[0].empty());
}

const std::vector<std::string>& TextBuffer::get_lines() const {
    return lines;
}
