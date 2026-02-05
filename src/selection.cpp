#include "selection.hpp"
#include <algorithm>

Selection::Selection() : active(false), start_x(0), start_y(0), end_x(0), end_y(0) {}

void Selection::start(int x, int y) {
    active = true;
    start_x = x;
    start_y = y;
    end_x = x;
    end_y = y;
}

void Selection::update(int x, int y) {
    if (active) {
        end_x = x;
        end_y = y;
    }
}

void Selection::clear() {
    active = false;
    start_x = 0;
    start_y = 0;
    end_x = 0;
    end_y = 0;
}

bool Selection::is_active() const {
    return active;
}

bool Selection::is_char_selected(int x, int y) const {
    if (!active) {
        return false;
    }
    
    // Normalize selection bounds
    int norm_start_y = std::min(start_y, end_y);
    int norm_end_y = std::max(start_y, end_y);
    int norm_start_x = start_x;
    int norm_end_x = end_x;
    
    // Handle single-line selection
    if (start_y == end_y) {
        norm_start_x = std::min(start_x, end_x);
        norm_end_x = std::max(start_x, end_x);
    } else if (start_y > end_y) {
        std::swap(norm_start_x, norm_end_x);
    }
    
    // Check if position is within selection
    if (y < norm_start_y || y > norm_end_y) {
        return false;
    }
    
    if (y == norm_start_y && y == norm_end_y) {
        return x >= norm_start_x && x < norm_end_x;
    }
    
    if (y == norm_start_y) {
        return x >= norm_start_x;
    }
    
    if (y == norm_end_y) {
        return x < norm_end_x;
    }
    
    return true;
}

void Selection::get_bounds(int& out_start_x, int& out_start_y, int& out_end_x, int& out_end_y) const {
    out_start_y = std::min(start_y, end_y);
    out_end_y = std::max(start_y, end_y);
    out_start_x = start_x;
    out_end_x = end_x;
    
    if (start_y == end_y) {
        out_start_x = std::min(start_x, end_x);
        out_end_x = std::max(start_x, end_x);
    } else if (start_y > end_y) {
        std::swap(out_start_x, out_end_x);
    }
}

void Selection::get_start(int& x, int& y) const {
    x = start_x;
    y = start_y;
}

void Selection::get_end(int& x, int& y) const {
    x = end_x;
    y = end_y;
}
