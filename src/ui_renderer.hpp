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
        std::function<bool(int, int)> is_char_selected_fn
    );

private:
    /// @brief Render the header bar
    ftxui::Element render_header(const std::string& filename, bool modified);
    
    /// @brief Render the status bar
    ftxui::Element render_status_bar(
        int cursor_x, int cursor_y,
        const std::string& status_message,
        bool save_status_shown
    );
    
    /// @brief Render the shortcuts bar
    ftxui::Element render_shortcuts();
    
    /// @brief Render the save button
    ftxui::Element render_save_button(bool modified);
    
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
