#include "cursor_manager.hpp"
#include "utf8_utils.hpp"
#include <algorithm>  // std::min, std::max
#include <cctype>     // isspace, isalnum

CursorManager::CursorManager() {}

void CursorManager::skip_formatting_markers(const std::string& line, int& cursor_x, int direction) {
    if (cursor_x < 0 || cursor_x > (int)line.length()) return;
    
    // Check for markdown markers and skip over them
    if (direction < 0) {  // Moving left
        // Check if we're at the end of a closing marker
        if (cursor_x >= 2 && line.compare(cursor_x - 2, 2, "**") == 0) {
            cursor_x -= 2;
        } else if (cursor_x >= 2 && line.compare(cursor_x - 2, 2, "~~") == 0) {
            cursor_x -= 2;
        } else if (cursor_x >= 4 && line.compare(cursor_x - 4, 4, "</u>") == 0) {
            cursor_x -= 4;
        } else if (cursor_x >= 1 && cursor_x < (int)line.length() && line[cursor_x - 1] == '*') {
            // Single * for italic - but avoid double-counting **
            if (!(cursor_x >= 2 && line[cursor_x - 2] == '*')) {
                cursor_x -= 1;
            }
        }
        // Check if we jumped onto an opening marker, skip that too
        if (cursor_x >= 2 && line.compare(cursor_x - 2, 2, "**") == 0) {
            cursor_x -= 2;
        } else if (cursor_x >= 2 && line.compare(cursor_x - 2, 2, "~~") == 0) {
            cursor_x -= 2;
        } else if (cursor_x >= 3 && line.compare(cursor_x - 3, 3, "<u>") == 0) {
            cursor_x -= 3;
        } else if (cursor_x >= 1 && line[cursor_x - 1] == '*') {
            if (!(cursor_x >= 2 && line[cursor_x - 2] == '*')) {
                cursor_x -= 1;
            }
        }
    } else {  // Moving right
        // Check if we're at the start of an opening marker
        if (cursor_x + 2 <= (int)line.length() && line.compare(cursor_x, 2, "**") == 0) {
            cursor_x += 2;
        } else if (cursor_x + 2 <= (int)line.length() && line.compare(cursor_x, 2, "~~") == 0) {
            cursor_x += 2;
        } else if (cursor_x + 3 <= (int)line.length() && line.compare(cursor_x, 3, "<u>") == 0) {
            cursor_x += 3;
        } else if (cursor_x < (int)line.length() && line[cursor_x] == '*') {
            // Single * for italic
            if (!(cursor_x + 1 < (int)line.length() && line[cursor_x + 1] == '*')) {
                cursor_x += 1;
            }
        }
        // Check if we jumped onto a closing marker, skip that too
        if (cursor_x + 2 <= (int)line.length() && line.compare(cursor_x, 2, "**") == 0) {
            cursor_x += 2;
        } else if (cursor_x + 2 <= (int)line.length() && line.compare(cursor_x, 2, "~~") == 0) {
            cursor_x += 2;
        } else if (cursor_x + 4 <= (int)line.length() && line.compare(cursor_x, 4, "</u>") == 0) {
            cursor_x += 4;
        } else if (cursor_x < (int)line.length() && line[cursor_x] == '*') {
            if (!(cursor_x + 1 < (int)line.length() && line[cursor_x + 1] == '*')) {
                cursor_x += 1;
            }
        }
    }
}

void CursorManager::move_left(
    const std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y,
    std::function<void()> update_selection_fn,
    std::function<void()> clear_selection_fn,
    bool select
) {
    // Move cursor first - move by UTF-8 character, not byte
    if (cursor_x > 0) {
        cursor_x = UTF8Utils::prev_char_boundary(buffer[cursor_y], cursor_x);
        // Only skip formatting markers when not selecting (allow selecting markers)
        if (!select) {
            skip_formatting_markers(buffer[cursor_y], cursor_x, -1);
        }
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
    std::function<void()> update_selection_fn,
    std::function<void()> clear_selection_fn,
    bool select
) {
    // Move cursor first - move by UTF-8 character, not byte
    if (cursor_x < (int)buffer[cursor_y].length()) {  // Cast size_t to int for comparison
        cursor_x = UTF8Utils::next_char_boundary(buffer[cursor_y], cursor_x);
        // Only skip formatting markers when not selecting (allow selecting markers)
        if (!select) {
            skip_formatting_markers(buffer[cursor_y], cursor_x, 1);
        }
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
    
    // Move to previous character boundary first
    size_t pos = UTF8Utils::prev_char_boundary(line, x);
    
    // Skip whitespace/non-word characters (check first byte of each UTF-8 char)
    while (pos > 0) {
        int char_len = UTF8Utils::get_char_length(line, pos);
        // Only treat single-byte ASCII as word characters
        if (char_len == 1 && (std::isalnum((unsigned char)line[pos]) || line[pos] == '_')) break;
        pos = UTF8Utils::prev_char_boundary(line, pos);
    }
    
    // Skip word characters
    while (pos > 0) {
        size_t prev = UTF8Utils::prev_char_boundary(line, pos);
        int char_len = UTF8Utils::get_char_length(line, prev);
        if (char_len == 1 && (std::isalnum((unsigned char)line[prev]) || line[prev] == '_')) {
            pos = prev;
        } else {
            break;
        }
    }
    
    return pos;
}

int CursorManager::find_word_end(const std::string& line, int x) {
    int len = line.length();
    
    if (x >= len) return len;
    
    // Skip whitespace/non-word characters using UTF-8 boundaries
    size_t pos = x;
    while (pos < (size_t)len) {
        int char_len = UTF8Utils::get_char_length(line, pos);
        if (char_len == 1 && (std::isalnum((unsigned char)line[pos]) || line[pos] == '_')) break;
        pos = UTF8Utils::next_char_boundary(line, pos);
    }
    
    // Skip word characters
    while (pos < (size_t)len) {
        int char_len = UTF8Utils::get_char_length(line, pos);
        if (char_len == 1 && (std::isalnum((unsigned char)line[pos]) || line[pos] == '_')) {
            pos = UTF8Utils::next_char_boundary(line, pos);
        } else {
            break;
        }
    }
    
    return pos;
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

bool CursorManager::is_cursor_inside_formatting_markers(const std::string& line, int cursor_x) {
    if (cursor_x < 0 || cursor_x > (int)line.length()) return false;
    
    // Check if we're inside ** (bold)
    if (cursor_x >= 2 && cursor_x < (int)line.length()) {
        // Check for opening ** before cursor
        size_t opening = line.rfind("**", cursor_x - 1);
        if (opening != std::string::npos && opening < (size_t)cursor_x) {
            // Check for closing ** at or after cursor
            size_t closing = line.find("**", cursor_x);
            if (closing != std::string::npos) {
                return true;
            }
        }
    }
    
    // Check if we're inside ~~ (strikethrough)
    if (cursor_x >= 2 && cursor_x < (int)line.length()) {
        size_t opening = line.rfind("~~", cursor_x - 1);
        if (opening != std::string::npos && opening < (size_t)cursor_x) {
            size_t closing = line.find("~~", cursor_x);
            if (closing != std::string::npos) {
                return true;
            }
        }
    }
    
    // Check if we're inside <u>...</u> (underline)
    if (cursor_x >= 3 && cursor_x < (int)line.length()) {
        size_t opening = line.rfind("<u>", cursor_x - 1);
        if (opening != std::string::npos && opening < (size_t)cursor_x) {
            size_t closing = line.find("</u>", cursor_x);
            if (closing != std::string::npos) {
                return true;
            }
        }
    }
    
    // Check if we're inside * (italic) - but exclude ** which we already checked
    if (cursor_x >= 1 && cursor_x < (int)line.length()) {
        size_t opening = line.rfind("*", cursor_x - 1);
        if (opening != std::string::npos && opening < (size_t)cursor_x) {
            // Make sure it's not part of **
            if (opening == 0 || line[opening - 1] != '*') {
                if (opening + 1 >= line.length() || line[opening + 1] != '*') {
                    size_t closing = line.find("*", cursor_x);
                    if (closing != std::string::npos) {
                        // Make sure closing is not part of **
                        if (closing == 0 || line[closing - 1] != '*') {
                            if (closing + 1 >= line.length() || line[closing + 1] != '*') {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return false;
}

void CursorManager::get_formatting_at_cursor(const std::string& line, int cursor_x, 
                                             bool& is_bold, bool& is_italic, 
                                             bool& is_underline, bool& is_strikethrough) {
    is_bold = false;
    is_italic = false;
    is_underline = false;
    is_strikethrough = false;
    
    if (cursor_x < 0 || cursor_x > (int)line.length()) return;
    
    // Check for bold **
    // Need to find the OPENING ** before cursor, not just any **
    if (cursor_x >= 2) {
        // Search backwards for **, but verify it's an opening by counting ** before it
        int bold_count = 0;
        size_t pos = 0;
        while (pos <= (size_t)cursor_x - 2) {
            size_t found = line.find("**", pos);
            if (found == std::string::npos || found >= (size_t)cursor_x) break;
            pos = found + 2;
            bold_count++;
        }
        // If odd number of ** before cursor, we're inside bold
        if (bold_count % 2 == 1) {
            // Verify there's a closing ** after cursor
            size_t closing = line.find("**", cursor_x);
            if (closing != std::string::npos) {
                is_bold = true;
            }
        }
    }
    
    // Check for strikethrough ~~
    if (cursor_x >= 2) {
        int strike_count = 0;
        size_t pos = 0;
        while (pos <= (size_t)cursor_x - 2) {
            size_t found = line.find("~~", pos);
            if (found == std::string::npos || found >= (size_t)cursor_x) break;
            pos = found + 2;
            strike_count++;
        }
        if (strike_count % 2 == 1) {
            size_t closing = line.find("~~", cursor_x);
            if (closing != std::string::npos) {
                is_strikethrough = true;
            }
        }
    }
    
    // Check for underline <u>
    if (cursor_x >= 3) {
        int underline_count = 0;
        size_t pos = 0;
        while (pos <= (size_t)cursor_x - 3) {
            size_t open_tag = line.find("<u>", pos);
            if (open_tag == std::string::npos || open_tag >= (size_t)cursor_x) break;
            pos = open_tag + 3;
            underline_count++;
        }
        // Count closing tags
        pos = 0;
        int close_count = 0;
        while (pos < (size_t)cursor_x) {
            size_t close_tag = line.find("</u>", pos);
            if (close_tag == std::string::npos || close_tag >= (size_t)cursor_x) break;
            pos = close_tag + 4;
            close_count++;
        }
        if (underline_count > close_count) {
            size_t closing = line.find("</u>", cursor_x);
            if (closing != std::string::npos) {
                is_underline = true;
            }
        }
    }
    
    // Check for italic * (but exclude **)
    if (cursor_x >= 1) {
        int italic_count = 0;
        size_t pos = 0;
        while (pos < (size_t)cursor_x) {
            size_t found = line.find("*", pos);
            if (found == std::string::npos || found >= (size_t)cursor_x) break;
            // Skip if it's part of **
            if (found + 1 < line.length() && line[found + 1] == '*') {
                pos = found + 2;
                continue;
            }
            if (found > 0 && line[found - 1] == '*') {
                pos = found + 1;
                continue;
            }
            pos = found + 1;
            italic_count++;
        }
        if (italic_count % 2 == 1) {
            // Find next * that's not part of **
            size_t search_pos = cursor_x;
            while (search_pos < line.length()) {
                size_t found = line.find("*", search_pos);
                if (found == std::string::npos) break;
                // Check if it's part of **
                bool part_of_double = false;
                if (found + 1 < line.length() && line[found + 1] == '*') {
                    part_of_double = true;
                }
                if (found > 0 && line[found - 1] == '*') {
                    part_of_double = true;
                }
                if (!part_of_double) {
                    is_italic = true;
                    break;
                }
                search_pos = found + 1;
            }
        }
    }
}
