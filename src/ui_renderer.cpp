#include "ui_renderer.hpp"
#include "utf8_utils.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/screen/terminal.hpp"
#include <algorithm>

using namespace ftxui;

// UI Color Constants - Disabled/Inactive State
static const Color BUTTON_DISABLED_BG_PRIMARY = Color::GrayDark;
static const Color BUTTON_DISABLED_BG_SECONDARY = Color::Black;
static const Color BUTTON_DISABLED_FG = Color::White;

// UI Color Constants - Active State
static const Color BUTTON_ACTIVE_FG = Color::Black;

// Button-Specific Colors (Active/Enabled State)
static const Color SAVE_BUTTON_ACTIVE_BG = Color::BlueLight;
static const Color BOLD_BUTTON_ACTIVE_BG = Color::Orange3;
static const Color ITALIC_BUTTON_ACTIVE_BG = Color::Pink3;
static const Color UNDERLINE_BUTTON_ACTIVE_BG = Color::Green4;
static const Color STRIKETHROUGH_BUTTON_ACTIVE_BG = Color::Red3;
static const Color BULLET_BUTTON_BG = Color::Black;
static const Color BULLET_BUTTON_FG = Color::White;
static const Color FONT_BUTTON_BG = Color::GrayDark;
static const Color FONT_BUTTON_FG = Color::White;
static const Color UNDO_BUTTON_ACTIVE_BG = Color::DarkOrange;
static const Color REDO_BUTTON_ACTIVE_BG = Color::GreenLight;
static const Color CLOSE_BUTTON_BG = Color::GrayLight;
static const Color CLOSE_BUTTON_FG = Color::RedLight;

UIRenderer::UIRenderer() {}

bool UIRenderer::supports_emojis() const {
    // The 'static' keyword ensures this block runs ONLY the first time the function is called.
    static const bool emoji_capable = []() -> bool {
        const char* colorterm = std::getenv("COLORTERM");
        const char* term = std::getenv("TERM");
        std::string term_str = term ? term : "";

        // 1. Check for TrueColor. Almost all modern terminals with 24-bit color 
        // handle font fallbacks and emojis well (Gnome, Alacritty, Kitty).
        if (colorterm && (std::string(colorterm) == "truecolor" || 
                          std::string(colorterm) == "24bit")) {
            return true;
        }

        // 2. Explicitly whitelist modern terminals that might not set COLORTERM
        if (term_str == "alacritty" || term_str == "xterm-kitty" || term_str == "foot") {
            return true;
        }

        // 3. Known "Legacy" environments (like UXTerm/Xterm)
        // These are best served with your '⌼' and other safe Unicode symbols.
        return false; 
    }();

    return emoji_capable;
}

UIRenderer::ParseResult UIRenderer::parse_markdown_segment(const std::string& line_text, size_t start_pos, bool is_selected, int cursor_x_in_line) {
    ParseResult result;
    result.bytes_consumed = 0;
    
    if (start_pos >= line_text.length()) {
        return result;
    }
    
    // Check for markdown markers
    std::string remaining = line_text.substr(start_pos);
    
    // Check for bold **
    if (remaining.length() >= 2 && remaining.substr(0, 2) == "**") {
        size_t end_pos = remaining.find("**", 2);
        if (end_pos != std::string::npos) {
            std::string content = remaining.substr(2, end_pos - 2);
            result.bytes_consumed = end_pos + 2;
            
            // Recursively parse content for nested formatting
            size_t content_pos = 0;
            while (content_pos < content.length()) {
                auto nested = parse_markdown_segment(content, content_pos, is_selected, 
                    cursor_x_in_line >= 0 ? cursor_x_in_line - (int)(start_pos + 2) : -1);
                
                // Apply bold to all nested elements
                for (auto& elem : nested.elements) {
                    elem = elem | bold;
                }
                result.elements.insert(result.elements.end(), nested.elements.begin(), nested.elements.end());
                content_pos += nested.bytes_consumed;
            }
            return result;
        }
    }
    
    // Check for strikethrough ~~
    if (remaining.length() >= 2 && remaining.substr(0, 2) == "~~") {
        size_t end_pos = remaining.find("~~", 2);
        if (end_pos != std::string::npos) {
            std::string content = remaining.substr(2, end_pos - 2);
            result.bytes_consumed = end_pos + 2;
            
            // Recursively parse content for nested formatting
            size_t content_pos = 0;
            while (content_pos < content.length()) {
                auto nested = parse_markdown_segment(content, content_pos, is_selected,
                    cursor_x_in_line >= 0 ? cursor_x_in_line - (int)(start_pos + 2) : -1);
                
                // Apply strikethrough to all nested elements
                for (auto& elem : nested.elements) {
                    elem = elem | strikethrough;
                }
                result.elements.insert(result.elements.end(), nested.elements.begin(), nested.elements.end());
                content_pos += nested.bytes_consumed;
            }
            return result;
        }
    }
    
    // Check for underline <u>
    if (remaining.length() >= 3 && remaining.substr(0, 3) == "<u>") {
        size_t end_pos = remaining.find("</u>");
        if (end_pos != std::string::npos) {
            std::string content = remaining.substr(3, end_pos - 3);
            result.bytes_consumed = end_pos + 4;
            
            // Recursively parse content for nested formatting
            size_t content_pos = 0;
            while (content_pos < content.length()) {
                auto nested = parse_markdown_segment(content, content_pos, is_selected,
                    cursor_x_in_line >= 0 ? cursor_x_in_line - (int)(start_pos + 3) : -1);
                
                // Apply underline to all nested elements
                for (auto& elem : nested.elements) {
                    elem = elem | underlined;
                }
                result.elements.insert(result.elements.end(), nested.elements.begin(), nested.elements.end());
                content_pos += nested.bytes_consumed;
            }
            return result;
        }
    }
    
    // Check for italic *
    if (remaining.length() >= 1 && remaining[0] == '*') {
        size_t end_pos = remaining.find("*", 1);
        if (end_pos != std::string::npos) {
            std::string content = remaining.substr(1, end_pos - 1);
            result.bytes_consumed = end_pos + 2;
            
            // Recursively parse content for nested formatting
            size_t content_pos = 0;
            while (content_pos < content.length()) {
                auto nested = parse_markdown_segment(content, content_pos, is_selected,
                    cursor_x_in_line >= 0 ? cursor_x_in_line - (int)(start_pos + 1) : -1);
                
                // Apply italic to all nested elements
                for (auto& elem : nested.elements) {
                    elem = elem | italic;
                }
                result.elements.insert(result.elements.end(), nested.elements.begin(), nested.elements.end());
                content_pos += nested.bytes_consumed;
            }
            return result;
        }
    }
    
    // No formatting found - return single character
    int char_len = UTF8Utils::get_char_length(line_text, start_pos);
    std::string ch_str = line_text.substr(start_pos, char_len);
    result.bytes_consumed = char_len;
    
    bool is_cursor_here = (cursor_x_in_line >= 0 && (int)start_pos == cursor_x_in_line);
    auto elem = text(ch_str);
    if (is_cursor_here) elem = elem | inverted | bold;
    if (is_selected) elem = elem | bgcolor(Color::Blue) | color(Color::Black);
    result.elements.push_back(elem);
    
    return result;
}

Element UIRenderer::render(
    const std::vector<std::string>& buffer,
    int cursor_x, int cursor_y,
    int scroll_y,
    const std::string& filename,
    bool modified,
    const std::string& status_message,
    bool save_status_shown,
    bool can_undo,
    bool can_redo,
    bool bold_active,
    bool italic_active,
    bool underline_active,
    bool strikethrough_active,
    std::function<bool(int, int)> is_char_selected_fn
) {
    int screen_height = Terminal::Size().dimy;
    int visible_lines = screen_height - 3;
    
    auto lines = render_lines(buffer, cursor_x, cursor_y, scroll_y, visible_lines, is_char_selected_fn);
    
    return vbox({
        render_header(filename, modified, can_undo, can_redo, bold_active, italic_active, underline_active, strikethrough_active),
        separator(),
        vbox(std::move(lines)) | flex,
        separator(),
        render_status_bar(cursor_x, cursor_y, status_message, save_status_shown),
        render_shortcuts()
    });
}

Elements UIRenderer::render_lines(
    const std::vector<std::string>& buffer,
    int cursor_x, int cursor_y,
    int scroll_y,
    int visible_lines,
    std::function<bool(int, int)> is_char_selected_fn
) {
    Elements lines_display;
    int max_line_num_width = std::to_string(buffer.size()).length();
    
    for (int i = 0; i < visible_lines && (scroll_y + i) < (int)buffer.size(); i++) {
        int line_idx = scroll_y + i;
        std::string line_num = std::to_string(line_idx + 1);
        
        // Pad line number
        while ((int)line_num.length() < max_line_num_width) {
            line_num = " " + line_num;
        }
        
        std::string line_content = buffer[line_idx];
        
        // Build line with selection highlighting and markdown parsing
        Elements line_elements;
        size_t byte_pos = 0;
        
        while (byte_pos <= line_content.length()) {
            if (byte_pos == line_content.length() && line_idx != cursor_y) break;
            
            bool is_selected = is_char_selected_fn(byte_pos, line_idx);
            
            if (byte_pos < line_content.length()) {
                // Handle tabs specially
                if (line_content[byte_pos] == '\t') {
                    bool is_cursor = (line_idx == cursor_y && (int)byte_pos == cursor_x);
                    auto elem = text("➡️   ");  // 4 spaces to represent a tab
                    if (is_cursor) elem = elem | inverted | bold;
                    else if (is_selected) elem = elem | bgcolor(Color::Blue) | color(Color::Black);
                    line_elements.push_back(elem);
                    byte_pos++;
                } else {
                    // Parse markdown and apply formatting
                    // Pass cursor_x if this is the cursor line, -1 otherwise
                    int cursor_x_for_parse = (line_idx == cursor_y) ? cursor_x : -1;
                    auto parse_result = parse_markdown_segment(line_content, byte_pos, is_selected, cursor_x_for_parse);
                    
                    for (auto& elem : parse_result.elements) {
                        line_elements.push_back(elem);
                    }
                    byte_pos += parse_result.bytes_consumed;
                    if (parse_result.bytes_consumed == 0) {
                        // Fallback - advance by one character to avoid infinite loop
                        byte_pos++;
                    }
                }
            } else {
                bool is_cursor = (line_idx == cursor_y && (int)byte_pos == cursor_x);
                auto elem = text(" ");
                if (is_cursor) elem = elem | inverted | bold;
                line_elements.push_back(elem);
                byte_pos++;
            }
        }
        
        // line number + separator + content
        auto line_elem = hbox(std::move(line_elements));
        auto full_line = hbox({
            text(line_num) | color(Color::GrayDark),
            text(" │ ") | color(Color::GrayDark),
            line_elem
        });
        
        lines_display.push_back(full_line);
    }
    
    return lines_display;
}

Element UIRenderer::render_header(const std::string& filename, bool modified, bool can_undo, bool can_redo,
                                  bool bold_active, bool italic_active, bool underline_active, bool strikethrough_active) {
    std::string title = "BZ-Nota - " + filename + (modified ? " [modified]" : "");
    return hbox({
        text(" "),
        render_save_button(modified),
        render_bold_button(bold_active),
        render_italic_button(italic_active),
        render_underline_button(underline_active),
        render_strikethrough_button(strikethrough_active),
        render_bullet_button(),
        //render_font_button(),
        text(" ") | flex,
        text(title) | bold | center, // center line, where title is displayed
        text(" ") | flex,
        render_undo_button(can_undo),
        render_redo_button(can_redo),
        render_close_button(),
        text(" ")
    }) | bgcolor(Color::DarkBlue);
}

Element UIRenderer::render_status_bar(
    int cursor_x, int cursor_y,
    const std::string& status_message,
    bool save_status_shown
) {
    std::string pos_info = "Line " + std::to_string(cursor_y + 1) + 
                          ", Col " + std::to_string(cursor_x + 1);
    std::string status_display = save_status_shown ? status_message : pos_info;
    
    return hbox({
        text(" " + status_display) | flex
    }) | bgcolor(Color::GrayDark);
}

Element UIRenderer::render_shortcuts() {
    return hbox({
        text(" ") | flex,
        text("Shift+arrow:Select by Char | Ctrl(Alt)+Shift+arrow:Select by Word | Ctrl+O:Insert new line above | Ctrl+K:Insert new line below") | center,
        text(" ") | flex
    }) | bgcolor(Color::Black);
}

Element UIRenderer::render_save_button(bool modified) {
    auto symbol = supports_emojis() ? text("💾") | nothing : text("⌼") | bold;
    return hbox({text(" "), symbol, text(" Ctrl+S ") | nothing}) | 
           bgcolor(modified ? SAVE_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_PRIMARY) |
           color(modified ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (modified ? bold : nothing);
}

Element UIRenderer::render_bold_button(bool active) {
    auto symbol = supports_emojis() ? text("🅱️") | nothing : text("B") | bold;
    return hbox({text(" "), symbol, text(" Ctrl+B ") | nothing}) | 
           bgcolor(active ? BOLD_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_SECONDARY) |
           color(active ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (active ? bold : nothing);
}

Element UIRenderer::render_italic_button(bool active) {
    auto symbol = text("I") | bold | italic;
    return hbox({text(" "), symbol, text(" Ctrl+I ")}) | 
           bgcolor(active ? ITALIC_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_PRIMARY) |
           color(active ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (active ? bold : nothing);
}

Element UIRenderer::render_underline_button(bool active) {
    auto symbol = text("U") | bold | underlined;
    return hbox({text(" "), symbol, text(" Ctrl+U ")}) | 
           bgcolor(active ? UNDERLINE_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_SECONDARY) |
           color(active ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (active ? bold : nothing);
}

Element UIRenderer::render_strikethrough_button(bool active) {
    auto symbol = text(" S ") | bold | strikethrough; // strike on the whole symbol for better visibility
    return hbox({symbol, text("Ctrl+T ")}) | 
           bgcolor(active ? STRIKETHROUGH_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_PRIMARY) |
           color(active ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (active ? bold : nothing);
}

Element UIRenderer::render_bullet_button() {
    auto symbol = text("•") | bold; // this works on all terminals and is visually distinct, so no need for emoji fallback
    return hbox({text(" "), symbol, text(" Alt+[0-9] ")}) | 
           bgcolor(BULLET_BUTTON_BG) |
           color(BULLET_BUTTON_FG);
}

// this requires more integration so I closed it for now
Element UIRenderer::render_font_button() {
    auto symbol = text("F") | bold;
    return hbox({text(" "), symbol, text(" Ctrl+F+Arrow ")}) | 
           bgcolor(FONT_BUTTON_BG) | 
           color(FONT_BUTTON_FG);
}

Element UIRenderer::render_undo_button(bool available) {
    auto symbol = text("↩️"); // This works for UXTerm or simple text fallback.
    return hbox({text(" "), symbol, text(" Ctrl+Z ")}) | 
           bgcolor(available ? UNDO_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_PRIMARY) |
           color(available ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (available ? bold : nothing);
}

Element UIRenderer::render_redo_button(bool available) {
    auto symbol = text("↪️"); // This works for UXTerm or simple text fallback.
    return hbox({text(" "), symbol, text(" Ctrl+Y ")}) | 
           bgcolor(available ? REDO_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_SECONDARY) |
           color(available ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (available ? bold : nothing);
}

Element UIRenderer::render_close_button() {
    auto symbol = supports_emojis() ? text("❌") | nothing : text("X") | bold;
    return hbox({text(" "), symbol, text(" Ctrl+Q ")}) | 
           bgcolor(CLOSE_BUTTON_BG) | 
           color(CLOSE_BUTTON_FG) | 
           bold;
}
