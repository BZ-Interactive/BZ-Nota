#include "ui_renderer.hpp"
#include "utf8_utils.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/screen/terminal.hpp"
//#include <shared_types.hpp>

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

// Status bar colors
static const Color STATUS_BAR_BG = Color::GrayDark;
static const Color STATUS_BAR_FG = Color::White;
static const Color STATUS_BAR_SUCCESS_BG = Color::GreenLight;
static const Color STATUS_BAR_SUCCESS_FG = Color::Black;
static const Color STATUS_BAR_ERROR_BG = Color::Red3Bis;
static const Color STATUS_BAR_ERROR_FG = Color::Black;
static const Color STATUS_BAR_WARNING_BG = Color::Yellow3Bis;
static const Color STATUS_BAR_WARNING_FG = Color::Black;


static const Color EDITOR_MODE_BASIC_BG = Color::White;
static const Color EDITOR_MODE_BASIC_FG = Color::Black;
static const Color EDITOR_MODE_FANCY_BG = Color::SeaGreen1Bis;
static const Color EDITOR_MODE_FANCY_FG = Color::Black;
static const Color EDITOR_MODE_CODE_BG = Color::Magenta2Bis;
static const Color EDITOR_MODE_CODE_FG = Color::White;
static const Color EDITOR_MODE_DOCUMENT_BG = Color::NavyBlue;
static const Color EDITOR_MODE_DOCUMENT_FG = Color::White;

UIRenderer::UIRenderer() {}

bool UIRenderer::supports_emojis() const {
    // The 'static' keyword ensures this block runs ONLY the first time the function is called.
    static const bool emoji_capable = []() -> bool {
        const char* colorterm = std::getenv("COLORTERM");
        const char* term = std::getenv("TERM");
        const char* wt_session = std::getenv("WT_SESSION");
        const char* wt_profile = std::getenv("WT_PROFILE_ID");
        std::string term_str = term ? term : "";

        // 1. Windows Terminal detection - supports emojis well
        if (wt_session || wt_profile) {
            return true;
        }

        // 2. Check for TrueColor. Almost all modern terminals with 24-bit color 
        // handle font fallbacks and emojis well (Gnome, Alacritty, Kitty).
        if (colorterm && (std::string(colorterm) == "truecolor" || 
                          std::string(colorterm) == "24bit")) {
            return true;
        }

        // 3. Explicitly whitelist modern terminals that might not set COLORTERM
        if (term_str == "alacritty" || term_str == "xterm-kitty" || term_str == "foot") {
            return true;
        }

        // 4. Known "Legacy" environments (like UXTerm/Xterm, Windows CMD)
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
    std::string title = "BZ-Nota - " + filename + (modified ? " [modified]" : "");
    return hbox({
        spacing,
        render_save_button(modified),
        editor_mode == EditorMode::FANCY || editor_mode == EditorMode::DOCUMENT ? render_bold_button(bold_active) : empty,
        editor_mode == EditorMode::FANCY || editor_mode == EditorMode::DOCUMENT ? render_italic_button(italic_active) : empty,
        editor_mode == EditorMode::FANCY || editor_mode == EditorMode::DOCUMENT ? render_underline_button(underline_active) : empty,
        editor_mode == EditorMode::FANCY || editor_mode == EditorMode::DOCUMENT ? render_strikethrough_button(strikethrough_active) : empty,
        render_bullet_button(),
        //render_font_button(),
        spacing | flex,
        text(title) | bold | center, // center line, where title is displayed
        spacing | flex,
        render_undo_button(can_undo),
        render_redo_button(can_redo),
        render_editor_mode_dropdown(editor_mode), // For now we only have FANCY mode, but this can be extended in the future.
        render_close_button(),
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

// this is where we can display available shortcuts or tips that doesn't fit on the upper bar.
// its created once on 
Element UIRenderer::render_shortcuts() {
    static const Element shortcuts = hbox({
        spacing | flex,
        text("Shift+arrow:Select by Char | Ctrl(Alt)+Shift+arrow:Select by Word | Ctrl+O:Insert new line above | Ctrl+K:Insert new line below") | center,
        spacing | flex
    }) | bgcolor(Color::GrayDark) | color(Color::White);
    return shortcuts;
}

Element UIRenderer::render_save_button(bool modified) {
    if (!save_button_) {
        std::string label = supports_emojis() ? "💾 Ctrl+S" : "⌼ Ctrl+S";
        save_button_ = std::make_unique<UIButton>(label, SAVE_BUTTON_ACTIVE_BG, BUTTON_DISABLED_BG_PRIMARY, false, nullptr);
    }
    
    save_button_->set_active(modified);
    return save_button_->render();
}

Element UIRenderer::render_bold_button(bool active) {
    if (!bold_button_) {
        std::string label = supports_emojis() ? "🅱️ Alt+B" : "Ⓑ Alt+B";
        bold_button_ = std::make_unique<UIButton>(label, BOLD_BUTTON_ACTIVE_BG, BUTTON_DISABLED_BG_SECONDARY, false, nullptr);
    }
    
    bold_button_->set_active(active);
    return bold_button_->render();
}

Element UIRenderer::render_italic_button(bool active) {
    if (!italic_button_) {
        italic_button_ = std::make_unique<UIButton>("I Alt+I", ITALIC_BUTTON_ACTIVE_BG, BUTTON_DISABLED_BG_PRIMARY, false, nullptr);
    }
    
    italic_button_->set_active(active);
    return italic_button_->render();
}

Element UIRenderer::render_underline_button(bool active) {
    if (!underline_button_) {
        underline_button_ = std::make_unique<UIButton>("U̲ Alt+U", UNDERLINE_BUTTON_ACTIVE_BG, BUTTON_DISABLED_BG_SECONDARY, false, nullptr);
    }
    
    underline_button_->set_active(active);
    return underline_button_->render();
}

Element UIRenderer::render_strikethrough_button(bool active) {
    if (!strikethrough_button_) {
        strikethrough_button_ = std::make_unique<UIButton>("x̶ Alt+T", STRIKETHROUGH_BUTTON_ACTIVE_BG, BUTTON_DISABLED_BG_PRIMARY, false, nullptr);
    }
    
    strikethrough_button_->set_active(active);
    return strikethrough_button_->render();
}

Element UIRenderer::render_bullet_button() {
    if (!bullet_button_) {
        bullet_button_ = std::make_unique<UIButton>("•", Color::White, BUTTON_DISABLED_BG_PRIMARY, false, nullptr);
    }
    
    return bullet_button_->render();
}

Element UIRenderer::render_font_button() {
    if (!font_button_) {
        font_button_ = std::make_unique<UIButton>("F", Color::White, BUTTON_DISABLED_BG_SECONDARY, false, nullptr);
    }
    
    return font_button_->render();
}

Element UIRenderer::render_undo_button(bool available) {
    if (!undo_button_) {
        std::string label = supports_emojis() ? "↩️ Ctrl+Z" : "↺ Ctrl+Z";
        undo_button_ = std::make_unique<UIButton>(label, UNDO_BUTTON_ACTIVE_BG, BUTTON_DISABLED_BG_PRIMARY, false, nullptr);
    }
    
    undo_button_->set_active(available);
    return undo_button_->render();
}

Element UIRenderer::render_redo_button(bool available) {
    if (!redo_button_) {
        std::string label = supports_emojis() ? "↪️ Ctrl+Y" : "↻ Ctrl+Y";
        redo_button_ = std::make_unique<UIButton>(label, REDO_BUTTON_ACTIVE_BG, BUTTON_DISABLED_BG_SECONDARY, false, nullptr);
    }
    
    redo_button_->set_active(available);
    return redo_button_->render();
}

// not really a dropdown consider and change to button if need be
Element UIRenderer::render_editor_mode_dropdown(EditorMode mode) {
    std::string mode_text;
    Color bg_color;
    Color fg_color;

    switch (mode) {
        case EditorMode::BASIC:
            mode_text = "Mode: Basic";
            bg_color = EDITOR_MODE_BASIC_BG;
            fg_color = EDITOR_MODE_BASIC_FG;
            break;
        case EditorMode::FANCY:
            mode_text = "Mode: Fancy";
            bg_color = EDITOR_MODE_FANCY_BG;
            fg_color = EDITOR_MODE_FANCY_FG;
            break;
        case EditorMode::CODE:
            mode_text = "Mode: Code";
            bg_color = EDITOR_MODE_CODE_BG;
            fg_color = EDITOR_MODE_CODE_FG;
            break;
        case EditorMode::DOCUMENT:
            mode_text = "Mode: Document";
            bg_color = EDITOR_MODE_DOCUMENT_BG;
            fg_color = EDITOR_MODE_DOCUMENT_FG;
            break;
    }

    return hbox({spacing, text(mode_text), text(" F7 ") | nothing}) |
           bgcolor(bg_color) |
           color(fg_color) |
           bold;
}

Element UIRenderer::render_close_button() {
    auto symbol = supports_emojis() ? text("❌") | nothing : text("X") | bold;
    return hbox({spacing, symbol, text(" Ctrl+Q ") | nothing}) | 
           bgcolor(CLOSE_BUTTON_BG) | 
           color(CLOSE_BUTTON_FG) | 
           bold;
}
