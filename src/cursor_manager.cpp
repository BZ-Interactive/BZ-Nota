#include "cursor_manager.hpp"
#include <algorithm>  // std::min, std::max
#include <cctype>     // isspace, isalnum

CursorManager::CursorManager() {}

void CursorManager::move_left(
    const std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y,
    [[maybe_unused]] std::function<void()> start_selection_fn,  // Unused - Editor handles it
    std::function<void()> update_selection_fn,
    std::function<void()> clear_selection_fn,
    bool select
) {
    // Move cursor first
    if (cursor_x > 0) {
        cursor_x--;  // In-place modification via reference
    } else if (cursor_y > 0) {
        cursor_y--;
        cursor_x = buffer[cursor_y].length();  // .length() returns size_t (unsigned)
    }
    
    // Update selection state (Editor already handles start_selection)
    if (select && update_selection_fn) {
        update_selection_fn();
    } else if (clear_selection_fn) {
        clear_selection_fn();
    }
}

void CursorManager::move_right(
    const std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y,
    [[maybe_unused]] std::function<void()> start_selection_fn,  // Unused - Editor handles it
    std::function<void()> update_selection_fn,
    std::function<void()> clear_selection_fn,
    bool select
) {
    // Move cursor first
    if (cursor_x < (int)buffer[cursor_y].length()) {  // Cast size_t to int for comparison
        cursor_x++;
    } else if (cursor_y < (int)buffer.size() - 1) {
        cursor_y++;
        cursor_x = 0;
    }
    
    // Update selection state (Editor already handles start_selection)
    if (select && update_selection_fn) {
        update_selection_fn();
    } else if (clear_selection_fn) {
        clear_selection_fn();
    }
}

void CursorManager::move_up(
    const std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y,
    [[maybe_unused]] std::function<void()> start_selection_fn,  // Unused - Editor handles it
    std::function<void()> update_selection_fn,
    std::function<void()> clear_selection_fn,
    bool select
) {
    // Move cursor first
    if (cursor_y > 0) {
        cursor_y--;
        cursor_x = std::min(cursor_x, (int)buffer[cursor_y].length());
    }
    
    // Update selection state (Editor already handles start_selection)
    if (select && update_selection_fn) {
        update_selection_fn();
    } else if (clear_selection_fn) {
        clear_selection_fn();
    }
}

void CursorManager::move_down(
    const std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y,
    [[maybe_unused]] std::function<void()> start_selection_fn,  // Unused - Editor handles it
    std::function<void()> update_selection_fn,
    std::function<void()> clear_selection_fn,
    bool select
) {
    // Move cursor first
    if (cursor_y < (int)buffer.size() - 1) {
        cursor_y++;
        cursor_x = std::min(cursor_x, (int)buffer[cursor_y].length());
    }
    
    // Update selection state (Editor already handles start_selection)
    if (select && update_selection_fn) {
        update_selection_fn();
    } else if (clear_selection_fn) {
        clear_selection_fn();
    }
}

void CursorManager::move_word_left(
    const std::vector<std::string>& buffer,
    int& cursor_x,
    int cursor_y,
    [[maybe_unused]] std::function<void()> start_selection_fn,  // Unused - Editor handles it
    std::function<void()> update_selection_fn,
    std::function<void()> clear_selection_fn,
    bool select
) {
    // Move cursor first
    int new_x = find_word_start(buffer[cursor_y], cursor_x);
    cursor_x = new_x;
    
    // Update selection state (Editor already handles start_selection)
    if (select && update_selection_fn) {
        update_selection_fn();
    } else if (clear_selection_fn) {
        clear_selection_fn();
    }
}

void CursorManager::move_word_right(
    const std::vector<std::string>& buffer,
    int& cursor_x,
    int cursor_y,
    [[maybe_unused]] std::function<void()> start_selection_fn,  // Unused - Editor handles it
    std::function<void()> update_selection_fn,
    std::function<void()> clear_selection_fn,
    bool select
) {
    // Move cursor first
    int new_x = find_word_end(buffer[cursor_y], cursor_x);
    cursor_x = new_x;
    
    // Update selection state (Editor already handles start_selection)
    if (select && update_selection_fn) {
        update_selection_fn();
    } else if (clear_selection_fn) {
        clear_selection_fn();
    }
}

int CursorManager::find_word_start(const std::string& line, int x) {
    if (x == 0) return 0;
    
    // Skip whitespace
    while (x > 0 && !std::isalnum(line[x - 1]) && line[x - 1] != '_') {
        x--;
    }
    
    // Skip word characters
    while (x > 0 && (std::isalnum(line[x - 1]) || line[x - 1] == '_')) {
        x--;
    }
    
    return x;
}

int CursorManager::find_word_end(const std::string& line, int x) {
    int len = line.length();
    
    if (x >= len) return len;
    
    // Skip whitespace
    while (x < len && !std::isalnum(line[x]) && line[x] != '_') {
        x++;
    }
    
    // Skip word characters
    while (x < len && (std::isalnum(line[x]) || line[x] == '_')) {
        x++;
    }
    
    return x;
}

void CursorManager::move_home(
    const std::vector<std::string>& buffer,
    int& cursor_x,
    int cursor_y,
    std::function<void()> update_selection_fn,
    std::function<void()> clear_selection_fn,
    bool select
) {
    // Smart Home: toggle between first non-whitespace and column 0
    const std::string& line = buffer[cursor_y];
    
    // Find first non-whitespace character
    int first_non_ws = 0;
    while (first_non_ws < (int)line.length() && 
           (line[first_non_ws] == ' ' || line[first_non_ws] == '\t')) {
        first_non_ws++;
    }
    
    // If already at first non-whitespace (or line is all whitespace), go to column 0
    // Otherwise go to first non-whitespace
    if (cursor_x == first_non_ws || first_non_ws == (int)line.length()) {
        cursor_x = 0;
    } else {
        cursor_x = first_non_ws;
    }
    
    // Update selection state
    if (select && update_selection_fn) {
        update_selection_fn();
    } else if (clear_selection_fn) {
        clear_selection_fn();
    }
}

void CursorManager::move_end(
    const std::vector<std::string>& buffer,
    int& cursor_x,
    int cursor_y,
    std::function<void()> update_selection_fn,
    std::function<void()> clear_selection_fn,
    bool select
) {
    // Move to end of line
    cursor_x = buffer[cursor_y].length();
    
    // Update selection state
    if (select && update_selection_fn) {
        update_selection_fn();
    } else if (clear_selection_fn) {
        clear_selection_fn();
    }
}

void CursorManager::ensure_cursor_visible(int cursor_y, int& scroll_y, int screen_height) {
    int visible_lines = screen_height - 3; // Account for header/status
    
    if (cursor_y < scroll_y) {
        scroll_y = cursor_y;
    } else if (cursor_y >= scroll_y + visible_lines) {
        scroll_y = cursor_y - visible_lines + 1;
    }
}

