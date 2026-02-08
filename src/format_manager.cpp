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
