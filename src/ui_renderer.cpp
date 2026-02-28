#include "ui_renderer.hpp"
#include "utf8_utils.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/screen/terminal.hpp"
//#include <shared_types.hpp>

using namespace ftxui;

// Status bar colors
static const Color STATUS_BAR_BG = Color::GrayDark;
static const Color STATUS_BAR_FG = Color::White;
static const Color STATUS_BAR_SUCCESS_BG = Color::GreenLight;
static const Color STATUS_BAR_SUCCESS_FG = Color::Black;
static const Color STATUS_BAR_ERROR_BG = Color::Red3Bis;
static const Color STATUS_BAR_ERROR_FG = Color::Black;
static const Color STATUS_BAR_WARNING_BG = Color::Yellow3Bis;
static const Color STATUS_BAR_WARNING_FG = Color::Black;


UIRenderer::UIRenderer() {}

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

Element UIRenderer::render(const RenderParams& params) {
    int screen_height = Terminal::Size().dimy;
    int visible_lines = screen_height - 3;
    
    auto lines = render_lines(params.buffer, params.cursor_x, params.cursor_y, params.scroll_y, visible_lines, params.is_char_selected_fn, params.editor_mode);
    
    return vbox({
        render_header(params.filename, params.modified, params.can_undo, params.can_redo, params.bold_active, params.italic_active, params.underline_active, params.strikethrough_active, params.editor_mode),
        separator(),
        vbox(std::move(lines)) | flex,
        separator(),
        render_status_bar(params.cursor_x, params.cursor_y, params.status_message, params.status_shown, params.status_type),
        render_shortcuts()
    });
}

Elements UIRenderer::render_lines(
    const std::vector<std::string>& buffer,
    int cursor_x, int cursor_y,
    int scroll_y,
    int visible_lines,
    std::function<bool(int, int)> is_char_selected_fn,
    EditorMode editor_mode
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
                    auto elem = text(tab_symbol);  // 4 spaces to represent a tab
                    if (is_cursor) elem = elem | inverted | bold;
                    else if (is_selected) elem = elem | bgcolor(Color::Blue) | color(Color::Black);
                    line_elements.push_back(elem);
                    byte_pos++;
                } else {
                    // Only parse markdown in FANCY and DOCUMENT modes
                    if (editor_mode == EditorMode::FANCY || editor_mode == EditorMode::DOCUMENT) {
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
                    } else {
                        // BASIC or CODE mode - render plain text without markdown parsing
                        int char_len = UTF8Utils::get_char_length(line_content, byte_pos);
                        std::string ch_str = line_content.substr(byte_pos, char_len);
                        bool is_cursor = (line_idx == cursor_y && (int)byte_pos == cursor_x);
                        auto elem = text(ch_str);
                        if (is_cursor) elem = elem | inverted | bold;
                        else if (is_selected) elem = elem | bgcolor(Color::Blue) | color(Color::Black);
                        line_elements.push_back(elem);
                        byte_pos += char_len;
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
                                  bool bold_active, bool italic_active, bool underline_active, bool strikethrough_active,
                                  EditorMode editor_mode) {
    // Update each button's state – they only rebuild their ftxui element when state actually changed.
    _btn_save.set_modified(modified);
    _btn_bold.set_active(bold_active);
    _btn_italic.set_active(italic_active);
    _btn_underline.set_active(underline_active);
    _btn_strikethrough.set_active(strikethrough_active);
    _btn_undo.set_available(can_undo);
    _btn_redo.set_available(can_redo);
    _btn_editor_mode.set_mode(editor_mode);

    bool show_format_buttons = (editor_mode == EditorMode::FANCY || editor_mode == EditorMode::DOCUMENT);

    std::string title = "BZ-Nota - " + filename + (modified ? " [modified]" : "");
    return hbox({
        spacing,
        _btn_save.get_element(),
        show_format_buttons ? _btn_bold.get_element()          : empty,
        show_format_buttons ? _btn_italic.get_element()        : empty,
        show_format_buttons ? _btn_underline.get_element()     : empty,
        show_format_buttons ? _btn_strikethrough.get_element() : empty,
        _btn_bullet.get_element(),
        spacing | flex,
        text(title) | bold | center,
        spacing | flex,
        _btn_undo.get_element(),
        _btn_redo.get_element(),
        _btn_editor_mode.get_element(),
        _btn_close.get_element(),
        spacing
    }) | bgcolor(Color::DarkBlue);
}

Element UIRenderer::render_status_bar(
    int cursor_x, int cursor_y,
    const std::string& status_message,
    bool status_shown, StatusBarType status_type
) {
    Color background_color;
    Color foreground_color;
    bool is_bold = false;
    switch (status_type)
    {
    case StatusBarType::SUCCESS:
        background_color = STATUS_BAR_SUCCESS_BG;
        foreground_color = STATUS_BAR_SUCCESS_FG;
        break;
    case StatusBarType::ERROR:
        background_color = STATUS_BAR_ERROR_BG;
        foreground_color = STATUS_BAR_ERROR_FG;
        is_bold = true;
        break;
    case StatusBarType::WARNING:
        background_color = STATUS_BAR_WARNING_BG;
        foreground_color = STATUS_BAR_WARNING_FG;
        is_bold = true;
        break;
    default: // not needed but just in case
        background_color = STATUS_BAR_BG;
        foreground_color = STATUS_BAR_FG;
        break;
    }

    std::string pos_info = "Line " + std::to_string(cursor_y + 1) + 
                          ", Col " + std::to_string(cursor_x + 1);
    std::string status_display = status_shown ? status_message : pos_info;
    
    return hbox({
        text(" " + status_display) | flex
    }) | bgcolor(background_color) |
         color(foreground_color)   |
         (is_bold ? bold : nothing);
}

Element UIRenderer::render_shortcuts() {
    return hbox({
        spacing | flex,
        text("Shift+arrow:Select by Char | Ctrl(Alt)+Shift+arrow:Select by Word | Ctrl+O:Insert new line above | Ctrl+K:Insert new line below") | center,
        spacing | flex
    }) | bgcolor(Color::Black);
}

