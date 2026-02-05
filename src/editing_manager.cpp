#include "editing_manager.hpp"

EditingManager::EditingManager() {}

void EditingManager::insert_char(
    std::vector<std::string>& buffer,
    int& cursor_x,
    int cursor_y,
    char c
) {
    buffer[cursor_y].insert(cursor_x, 1, c);
    cursor_x++;
}

void EditingManager::insert_newline(
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y
) {
    std::string current_line = buffer[cursor_y];
    std::string before_cursor = current_line.substr(0, cursor_x);
    std::string after_cursor = current_line.substr(cursor_x);
    
    buffer[cursor_y] = before_cursor;
    buffer.insert(buffer.begin() + cursor_y + 1, after_cursor);
    
    cursor_y++;
    cursor_x = 0;
}

void EditingManager::delete_char(
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y
) {
    if (cursor_x > 0) {
        buffer[cursor_y].erase(cursor_x - 1, 1);
        cursor_x--;
    } else if (cursor_y > 0) {
        cursor_x = buffer[cursor_y - 1].length();
        buffer[cursor_y - 1] += buffer[cursor_y];
        buffer.erase(buffer.begin() + cursor_y);
        cursor_y--;
    }
}

void EditingManager::delete_forward(
    std::vector<std::string>& buffer,
    int cursor_x,
    int cursor_y
) {
    if (cursor_x < (int)buffer[cursor_y].length()) {
        buffer[cursor_y].erase(cursor_x, 1);
    } else if (cursor_y < (int)buffer.size() - 1) {
        buffer[cursor_y] += buffer[cursor_y + 1];
        buffer.erase(buffer.begin() + cursor_y + 1);
    }
}
