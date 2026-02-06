#include "ui_renderer.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/screen/terminal.hpp"
#include <algorithm>

using namespace ftxui;

UIRenderer::UIRenderer() {}

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
    std::function<bool(int, int)> is_char_selected_fn
) {
    int screen_height = Terminal::Size().dimy;
    int visible_lines = screen_height - 3;
    
    auto lines = render_lines(buffer, cursor_x, cursor_y, scroll_y, visible_lines, is_char_selected_fn);
    
    return vbox({
        render_header(filename, modified, can_undo, can_redo),
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
        
        // Replace tabs with visual representation
        size_t tab_pos = 0;
        while ((tab_pos = line_content.find('\t', tab_pos)) != std::string::npos) {
            line_content.replace(tab_pos, 1, "→   ");
            tab_pos += 4;
        }
        
        // Build line with selection highlighting
        Elements line_elements;
        for (int j = 0; j <= (int)line_content.length(); j++) {
            if (j == (int)line_content.length() && line_idx != cursor_y) break;
            
            bool is_cursor = (line_idx == cursor_y && j == cursor_x);
            bool is_selected = is_char_selected_fn(j, line_idx);
            
            std::string ch_str = (j < (int)line_content.length()) ? 
                                std::string(1, line_content[j]) : " ";
            
            auto elem = text(ch_str);
            
            if (is_cursor) {
                elem = elem | inverted | bold;
            } else if (is_selected) {
                elem = elem | bgcolor(Color::Blue) | color(Color::Black);
            }
            
            line_elements.push_back(elem);
        }
        
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

Element UIRenderer::render_header(const std::string& filename, bool modified, bool can_undo, bool can_redo) {
    std::string title = "BZ-Nota - " + filename + (modified ? " [modified]" : "");
    return hbox({
        text(" "),
        render_save_button(modified),
        render_undo_button(can_undo),
        render_redo_button(can_redo),
        text("  ") | flex,
        text(title) | bold | center,
        text("  ") | flex,
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

Element UIRenderer::render_undo_button(bool available) {
    return text(" ↩️ Ctrl+Z ") | 
           bgcolor(available ? Color(Color::DarkOrange) : Color(Color::GrayDark)) |
           color(available ? Color(Color::Black) : Color(Color::White)) |
           (available ? bold : nothing);
}

Element UIRenderer::render_redo_button(bool available) {
    return text(" ↪️ Ctrl+Y ") | 
           bgcolor(available ? Color(Color::GreenLight) : Color(Color::GrayDark)) |
           color(available ? Color(Color::Black) : Color(Color::White)) |
           (available ? bold : nothing);
}

Element UIRenderer::render_save_button(bool modified) {
    return text(" 💾 Ctrl+S ") | 
           bgcolor(modified ? Color(Color::BlueLight) : Color(Color::GrayDark)) |
           color(modified ? Color(Color::Black) : Color(Color::White)) |
           (modified ? bold : nothing);
}

Element UIRenderer::render_close_button() {
    return text(" ❌ Ctrl+Q ") | 
           bgcolor(Color::GrayLight) | 
           color(Color::RedLight) | 
           bold;
}
