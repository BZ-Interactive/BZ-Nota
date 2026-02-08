#include "selection_manager.hpp"
#include "formatter.hpp"
#include <algorithm>  // For std::min, std::max, std::swap

SelectionManager::SelectionManager() {}

void SelectionManager::start_selection(int cursor_x, int cursor_y) {
    has_selection = true;
    selection_start_x = cursor_x;
    selection_start_y = cursor_y;
    selection_end_x = cursor_x;
    selection_end_y = cursor_y;
}

void SelectionManager::update_selection(int cursor_x, int cursor_y) {
    if (has_selection) {
        selection_end_x = cursor_x;
        selection_end_y = cursor_y;
    }
}

void SelectionManager::clear_selection() {
    has_selection = false;
}

void SelectionManager::select_all(int end_x, int end_y) {
    has_selection = true;
    selection_start_x = 0;
    selection_start_y = 0;
    selection_end_x = end_x;
    selection_end_y = end_y;
}

void SelectionManager::delete_selection(
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y
) {
    if (!has_selection) return;
    
    // std::min/std::max
    int start_y = std::min(selection_start_y, selection_end_y);
    int end_y = std::max(selection_start_y, selection_end_y);
    int start_x = selection_start_x;
    int end_x = selection_end_x;
    
    if (selection_start_y > selection_end_y || 
        (selection_start_y == selection_end_y && selection_start_x > selection_end_x)) {
        std::swap(start_x, end_x);
    }
    
    if (start_y == end_y) {
        // Single line deletion
        buffer[start_y].erase(start_x, end_x - start_x);
        cursor_x = start_x;
        cursor_y = start_y;
    } else {
        // Multi-line deletion - string concatenation using +
        std::string remaining = buffer[start_y].substr(0, start_x) +
                               buffer[end_y].substr(end_x);
        buffer[start_y] = remaining;
        // vector.erase with iterators
        buffer.erase(buffer.begin() + start_y + 1, buffer.begin() + end_y + 1);
        cursor_x = start_x;
        cursor_y = start_y;
    }
    
    clear_selection();
}

std::string SelectionManager::get_selected_text(const std::vector<std::string>& buffer) const {
    if (!has_selection) return "";
    
    int start_y = std::min(selection_start_y, selection_end_y);
    int end_y = std::max(selection_start_y, selection_end_y);
    int start_x = selection_start_x;
    int end_x = selection_end_x;
    
    if (selection_start_y > selection_end_y || 
        (selection_start_y == selection_end_y && selection_start_x > selection_end_x)) {
        std::swap(start_x, end_x);
    }
    
    if (start_y == end_y) {
        // Single line
        return buffer[start_y].substr(start_x, end_x - start_x);
    } else {
        // Multi-line
        std::string result = buffer[start_y].substr(start_x) + "\n";
        for (int y = start_y + 1; y < end_y; y++) {
            result += buffer[y] + "\n";
        }
        result += buffer[end_y].substr(0, end_x);
        return result;
    }
}

bool SelectionManager::is_char_selected(int x, int y) const {
    if (!has_selection) return false;
    
    int start_y = std::min(selection_start_y, selection_end_y);
    int end_y = std::max(selection_start_y, selection_end_y);
    int start_x = selection_start_x;
    int end_x = selection_end_x;
    
    if (selection_start_y > selection_end_y || 
        (selection_start_y == selection_end_y && selection_start_x > selection_end_x)) {
        std::swap(start_x, end_x);
    }
    
    if (y < start_y || y > end_y) return false;
    if (y == start_y && y == end_y) {
        return x >= start_x && x < end_x;
    }
    if (y == start_y) {
        return x >= start_x;
    }
    if (y == end_y) {
        return x < end_x;
    }
    return true;
}

void SelectionManager::get_bounds(int& start_x, int& start_y, int& end_x, int& end_y) const {
    start_x = selection_start_x;
    start_y = selection_start_y;
    end_x = selection_end_x;
    end_y = selection_end_y;
}

void SelectionManager::adjust_selection_for_formatting(const std::vector<std::string>& buffer) {
    if (!has_selection) return;
    
    // Determine which is the start position
    int start_x = selection_start_x;
    int start_y = selection_start_y;
    int end_x = selection_end_x;
    int end_y = selection_end_y;
    
    if (selection_start_y > selection_end_y || 
        (selection_start_y == selection_end_y && selection_start_x > selection_end_x)) {
        std::swap(start_x, end_x);
        std::swap(start_y, end_y);
    }
    
    // Only adjust single-line selections for now
    if (start_y != end_y || start_y >= (int)buffer.size()) return;
    
    const std::string& line = buffer[start_y];
    
    // Use the object-oriented formatter approach
    adjust_selection_bounds(line, start_x, end_x);
    
    // Update the selection bounds
    if (selection_start_y == start_y && 
        (selection_start_y < selection_end_y || 
         (selection_start_y == selection_end_y && selection_start_x <= selection_end_x))) {
        selection_start_x = start_x;
        selection_end_x = end_x;
    } else {
        selection_end_x = start_x;
        selection_start_x = end_x;
    }
}
