#include "clipboard_manager.hpp"

ClipboardManager::ClipboardManager() {}

void ClipboardManager::copy(const std::string& text) {
    clipboard = text;
}

int ClipboardManager::cut(const std::string& text) {
    clipboard = text;
    return clipboard.size();
}

int ClipboardManager::paste(
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y
) {
    if (clipboard.empty()) return 0;
    
    size_t pos = 0;
    size_t newline_pos;
    bool first_line = true;
    
    while ((newline_pos = clipboard.find('\n', pos)) != std::string::npos) {
        std::string line_to_insert = clipboard.substr(pos, newline_pos - pos);
        
        if (first_line) {
            buffer[cursor_y].insert(cursor_x, line_to_insert);
            cursor_x += line_to_insert.length();
            first_line = false;
        } else {
            std::string remainder = buffer[cursor_y].substr(cursor_x);
            buffer[cursor_y] = buffer[cursor_y].substr(0, cursor_x);
            cursor_y++;
            buffer.insert(buffer.begin() + cursor_y, line_to_insert + remainder);
            cursor_x = line_to_insert.length();
        }
        
        pos = newline_pos + 1;
    }
    
    // Insert remaining text (no newline at end)
    if (pos < clipboard.length()) {
        std::string remaining = clipboard.substr(pos);
        buffer[cursor_y].insert(cursor_x, remaining);
        cursor_x += remaining.length();
    }
    
    return clipboard.size();
}
