#include "format_manager.hpp"

FormatManager::FormatManager() {}

// ===== Formatting Toggle Operations =====

void FormatManager::toggle_bold() {
    bold_active = !bold_active;
    status_message = bold_active ? "Bold enabled" : "Bold disabled";
}

void FormatManager::toggle_italic() {
    italic_active = !italic_active;
    status_message = italic_active ? "Italic enabled" : "Italic disabled";
}

void FormatManager::toggle_underline() {
    underline_active = !underline_active;
    status_message = underline_active ? "Underline enabled" : "Underline disabled";
}

void FormatManager::toggle_strikethrough() {
    strikethrough_active = !strikethrough_active;
    status_message = strikethrough_active ? "Strikethrough enabled" : "Strikethrough disabled";
}

std::string FormatManager::get_opening_markers() const {
    std::string markers;
    
    if (bold_active) markers += "**";
    if (italic_active) markers += "*";
    if (underline_active) markers += "<u>";
    if (strikethrough_active) markers += "~~";
    
    return markers;
}

std::string FormatManager::get_closing_markers() const {
    std::string markers;
    
    // Closing markers in reverse order
    if (strikethrough_active) markers += "~~";
    if (underline_active) markers += "</u>";
    if (italic_active) markers += "*";
    if (bold_active) markers += "**";
    
    return markers;
}

void FormatManager::start_formatting_session(std::vector<std::string>& buffer, int& cursor_x, int cursor_y) {
    if (!has_active_formatting() || session_active) {
        return;
    }
    
    std::string markers = get_opening_markers();
    buffer[cursor_y].insert(cursor_x, markers);
    cursor_x += markers.length();
    session_active = true;
}

void FormatManager::end_formatting_session(std::vector<std::string>& buffer, int& cursor_x, int cursor_y) {
    if (!session_active) {
        return;
    }
    
    std::string markers = get_closing_markers();
    buffer[cursor_y].insert(cursor_x, markers);
    cursor_x += markers.length();
    session_active = false;
}

void FormatManager::insert_formatting_markers(std::vector<std::string>& buffer, int& cursor_x, int cursor_y) {
    if (!has_active_formatting()) {
        return;
    }
    
    std::string opening = get_opening_markers();
    std::string closing = get_closing_markers();
    
    // Insert both markers at cursor position
    buffer[cursor_y].insert(cursor_x, opening + closing);
    // Move cursor to between the markers
    cursor_x += opening.length();
}

void FormatManager::split_formatting_at_cursor(std::vector<std::string>& buffer, int& cursor_x, int cursor_y, const std::string& format_type) {
    if (cursor_y >= (int)buffer.size()) return;
    std::string& line = buffer[cursor_y];
    if (cursor_x < 0 || cursor_x > (int)line.length()) return;
    
    std::string opening_marker, closing_marker;
    
    if (format_type == "bold") {
        opening_marker = "**";
        closing_marker = "**";
    } else if (format_type == "italic") {
        opening_marker = "*";
        closing_marker = "*";
    } else if (format_type == "underline") {
        opening_marker = "<u>";
        closing_marker = "</u>";
    } else if (format_type == "strikethrough") {
        opening_marker = "~~";
        closing_marker = "~~";
    } else {
        return;
    }
    
    // Find opening marker before cursor
    size_t opening_pos = line.rfind(opening_marker, cursor_x - 1);
    if (opening_pos == std::string::npos || opening_pos >= (size_t)cursor_x) return;
    
    // Find closing marker after cursor
    size_t closing_pos = line.find(closing_marker, cursor_x);
    if (closing_pos == std::string::npos || closing_pos <= (size_t)cursor_x) return;
    
    // Insert closing marker before cursor and opening marker after cursor
    // Insert in reverse order to not mess up positions
    line.insert(cursor_x, closing_marker + opening_marker);
    cursor_x += closing_marker.length();
}

std::string FormatManager::wrap_text(const std::string& text) const {
    if (text.empty() || !has_active_formatting()) {
        return text;
    }
    
    std::string result = text;
    
    // Apply formatting in order: bold, italic, underline, strikethrough
    // This creates nested formatting like **_~~text~~_**
    
    if (strikethrough_active) {
        result = "~~" + result + "~~";
    }
    
    if (underline_active) {
        // Using HTML for underline as markdown doesn't have native underline
        result = "<u>" + result + "</u>";
    }
    
    if (italic_active) {
        result = "*" + result + "*";
    }
    
    if (bold_active) {
        result = "**" + result + "**";
    }
    
    return result;
}

std::string FormatManager::wrap_with_bold(const std::string& text) const {
    return "**" + text + "**";
}

std::string FormatManager::wrap_with_italic(const std::string& text) const {
    return "*" + text + "*";
}

std::string FormatManager::wrap_with_underline(const std::string& text) const {
    return "<u>" + text + "</u>";
}

std::string FormatManager::wrap_with_strikethrough(const std::string& text) const {
    return "~~" + text + "~~";
}

int FormatManager::move_cursor_before_opening_markers(const std::string& line, int& cursor_x) const {
    if (cursor_x == 0) return 0;
    
    int original_x = cursor_x;
    bool moved = true;
    
    // Keep moving back while we find opening markers
    while (moved && cursor_x > 0) {
        moved = false;
        
        // Check for opening bold **
        if (cursor_x >= 2 && line.substr(cursor_x - 2, 2) == "**") {
            cursor_x -= 2;
            moved = true;
            continue;
        }
        
        // Check for opening underline <u>
        if (cursor_x >= 3 && line.substr(cursor_x - 3, 3) == "<u>") {
            cursor_x -= 3;
            moved = true;
            continue;
        }
        
        // Check for opening strikethrough ~~
        if (cursor_x >= 2 && line.substr(cursor_x - 2, 2) == "~~") {
            cursor_x -= 2;
            moved = true;
            continue;
        }
        
        // Check for opening italic * (but not **)
        if (cursor_x >= 1 && line[cursor_x - 1] == '*') {
            // Make sure it's not part of **
            if (cursor_x >= 2 && line[cursor_x - 2] == '*') {
                // This is part of **, skip it (already handled above)
            } else {
                cursor_x -= 1;
                moved = true;
                continue;
            }
        }
    }
    
    return original_x - cursor_x;
}

std::string FormatManager::extract_formatting_from_text(const std::string& text, bool& has_bold, bool& has_italic, bool& has_underline, bool& has_strikethrough) const {
    has_bold = false;
    has_italic = false;
    has_underline = false;
    has_strikethrough = false;
    
    std::string result = text;
    bool changed = true;
    
    // Keep removing formatting markers until none are left
    while (changed) {
        changed = false;
        
        // Remove bold **
        size_t bold_open = result.find("**");
        if (bold_open != std::string::npos) {
            size_t bold_close = result.find("**", bold_open + 2);
            if (bold_close != std::string::npos) {
                // Remove complete bold markers
                result = result.substr(0, bold_open) + result.substr(bold_open + 2, bold_close - bold_open - 2) + result.substr(bold_close + 2);
                has_bold = true;
                changed = true;
                continue;
            } else {
                // Remove incomplete opening marker
                result = result.substr(0, bold_open) + result.substr(bold_open + 2);
                has_bold = true;
                changed = true;
                continue;
            }
        }
        
        // Remove underline <u>
        size_t underline_open = result.find("<u>");
        if (underline_open != std::string::npos) {
            size_t underline_close = result.find("</u>", underline_open + 3);
            if (underline_close != std::string::npos) {
                // Remove complete underline markers
                result = result.substr(0, underline_open) + result.substr(underline_open + 3, underline_close - underline_open - 3) + result.substr(underline_close + 4);
                has_underline = true;
                changed = true;
                continue;
            } else {
                // Remove incomplete opening marker
                result = result.substr(0, underline_open) + result.substr(underline_open + 3);
                has_underline = true;
                changed = true;
                continue;
            }
        }
        
        // Remove strikethrough ~~
        size_t strike_open = result.find("~~");
        if (strike_open != std::string::npos) {
            size_t strike_close = result.find("~~", strike_open + 2);
            if (strike_close != std::string::npos) {
                // Remove complete strikethrough markers
                result = result.substr(0, strike_open) + result.substr(strike_open + 2, strike_close - strike_open - 2) + result.substr(strike_close + 2);
                has_strikethrough = true;
                changed = true;
                continue;
            } else {
                // Remove incomplete opening marker
                result = result.substr(0, strike_open) + result.substr(strike_open + 2);
                has_strikethrough = true;
                changed = true;
                continue;
            }
        }
        
        // Remove italic * (but not bold **)
        size_t italic_pos = result.find('*');
        if (italic_pos != std::string::npos) {
            // Make sure it's not part of **
            if (italic_pos + 1 < result.length() && result[italic_pos + 1] == '*') {
                // This is **, skip it (already handled above)
            } else {
                size_t italic_close = result.find('*', italic_pos + 1);
                if (italic_close != std::string::npos) {
                    // Remove complete italic markers
                    result = result.substr(0, italic_pos) + result.substr(italic_pos + 1, italic_close - italic_pos - 1) + result.substr(italic_close + 1);
                    has_italic = true;
                    changed = true;
                    continue;
                } else {
                    // Remove incomplete opening marker
                    result = result.substr(0, italic_pos) + result.substr(italic_pos + 1);
                    has_italic = true;
                    changed = true;
                    continue;
                }
            }
        }
    }
    
    // Clean up any orphaned/incomplete formatting markers that might remain
    // Remove orphaned closing tags </u>
    size_t orphan_pos;
    while ((orphan_pos = result.find("</u>")) != std::string::npos) {
        result = result.substr(0, orphan_pos) + result.substr(orphan_pos + 4);
        has_underline = true;
    }
    // Remove partial </u (without >)
    while ((orphan_pos = result.find("</u")) != std::string::npos) {
        size_t end_pos = orphan_pos + 3;
        while (end_pos < result.length() && result[end_pos] != ' ' && result[end_pos] != '\n') {
            end_pos++;
            if (result[end_pos - 1] == '>') break;
        }
        result = result.substr(0, orphan_pos) + result.substr(end_pos);
        has_underline = true;
    }
    // Remove orphaned opening tags <u>
    while ((orphan_pos = result.find("<u>")) != std::string::npos) {
        result = result.substr(0, orphan_pos) + result.substr(orphan_pos + 3);
        has_underline = true;
    }
    
    // Clean up orphaned ** (single occurrence)
    orphan_pos = result.find("**");
    if (orphan_pos != std::string::npos) {
        result = result.substr(0, orphan_pos) + result.substr(orphan_pos + 2);
        has_bold = true;
    }
    
    // Clean up orphaned ~~ (single occurrence)
    orphan_pos = result.find("~~");
    if (orphan_pos != std::string::npos) {
        result = result.substr(0, orphan_pos) + result.substr(orphan_pos + 2);
        has_strikethrough = true;
    }
    
    return result;
}

