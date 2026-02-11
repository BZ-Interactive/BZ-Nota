#pragma once
#include <string>
#include <vector>
#include "ftxui/dom/elements.hpp"
#include "shared_types.hpp"

/// @brief Handles all UI rendering for the editor
class UIRenderer {
public:
    UIRenderer();
    
    /// @brief Render the complete editor UI
    /// @param params All rendering parameters bundled in a struct
    /// @return The rendered FTXUI Element
    ftxui::Element render(const RenderParams& params);

private:
    /// @brief Check if the terminal supports emojis
    /// @return True if emojis are supported, false otherwise
    bool supports_emojis() const;
    
    /// @brief Parse markdown formatting from text and apply FTXUI styles
    /// @param text The text to parse (may contain markdown)
    /// @param start_pos Starting position in the text
    /// @param is_selected Whether this text is selected
    /// @param cursor_x_in_line Cursor X position in line (-1 if not on this line)
    /// @return Vector of styled elements and the number of bytes consumed
    struct ParseResult {
        ftxui::Elements elements;
        size_t bytes_consumed;
    };
    ParseResult parse_markdown_segment(const std::string& text, size_t start_pos, bool is_selected, int cursor_x_in_line);

    /// @brief Render the header bar
    ftxui::Element render_header(const std::string& filename, bool modified, bool can_undo, bool can_redo,
                                 bool bold_active, bool italic_active, bool underline_active, bool strikethrough_active,
                                 EditorMode editor_mode);
    
    /// @brief Render the status bar
    ftxui::Element render_status_bar(
        int cursor_x, int cursor_y,
        const std::string& status_message,
        bool status_shown, StatusBarType status_type
    );
    
    /// @brief Render the shortcuts bar, the bar below the writing area.
    ///
    ///        This is where we can display available shortcuts or tips that doesn't fit on the upper bar.
    ftxui::Element render_shortcuts();

    ftxui::Element render_save_button(bool modified);
    ftxui::Element render_bold_button(bool active);
    ftxui::Element render_italic_button(bool active);
    ftxui::Element render_underline_button(bool active);
    ftxui::Element render_strikethrough_button(bool active);
    ftxui::Element render_bullet_button();
    ftxui::Element render_font_button();
    ftxui::Element render_undo_button(bool available);
    ftxui::Element render_redo_button(bool available);
    
    ftxui::Element render_editor_mode_dropdown(EditorMode mode);

    /// @brief Render the close button
    ftxui::Element render_close_button();
    
    /// @brief Render the text lines with line numbers and selection
    ftxui::Elements render_lines(
        const std::vector<std::string>& buffer,
        int cursor_x, int cursor_y,
        int scroll_y,
        int visible_lines,
        std::function<bool(int, int)> is_char_selected_fn,
        EditorMode editor_mode
    );
};
