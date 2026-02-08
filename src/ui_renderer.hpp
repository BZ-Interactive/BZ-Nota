#pragma once
#include <string>
#include <vector>
#include "ftxui/dom/elements.hpp"

/// @brief Handles all UI rendering for the editor
class UIRenderer {
public:
    UIRenderer();
    
    /// @brief Render the complete editor UI
    /// @param buffer The text buffer lines
    /// @param cursor_x Current cursor X position
    /// @param cursor_y Current cursor Y position
    /// @param scroll_y Current scroll position
    /// @param filename Current filename
    /// @param modified Whether the file has been modified
    /// @param status_message Status message to display
    /// @param save_status_shown Whether to show status message
    /// @param can_undo Whether undo is available
    /// @param can_redo Whether redo is available
    /// @param bold_active Whether bold formatting is active
    /// @param italic_active Whether italic formatting is active
    /// @param underline_active Whether underline formatting is active
    /// @param strikethrough_active Whether strikethrough formatting is active
    /// @param is_char_selected_fn Function to check if a character is selected
    /// @return The rendered FTXUI Element
    ftxui::Element render(
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
    );

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
                                 bool bold_active, bool italic_active, bool underline_active, bool strikethrough_active);
    
    /// @brief Render the status bar
    ftxui::Element render_status_bar(
        int cursor_x, int cursor_y,
        const std::string& status_message,
        bool save_status_shown
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
    
    /// @brief Render the close button
    ftxui::Element render_close_button();
    
    /// @brief Render the text lines with line numbers and selection
    ftxui::Elements render_lines(
        const std::vector<std::string>& buffer,
        int cursor_x, int cursor_y,
        int scroll_y,
        int visible_lines,
        std::function<bool(int, int)> is_char_selected_fn
    );
};
