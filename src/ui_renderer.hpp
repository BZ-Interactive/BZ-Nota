#pragma once
#include <string>
#include <vector>
#include "ftxui/dom/elements.hpp"
#include "shared_types.hpp"
#include "ui_button.hpp"

/// @brief Handles all UI rendering for the editor
class UIRenderer {
public:
    UIRenderer();

    /// @brief Render the complete editor UI
    /// @param params All rendering parameters bundled in a struct
    /// @return The rendered FTXUI Element
    ftxui::Element render(const RenderParams& params);

private:
    // -----------------------------------------------------------------------
    // Cached header-bar buttons (rebuilt only when their state changes)
    // -----------------------------------------------------------------------
    SaveButton          _btn_save;
    BoldButton          _btn_bold;
    ItalicButton        _btn_italic;
    UnderlineButton     _btn_underline;
    StrikethroughButton _btn_strikethrough;
    BulletButton        _btn_bullet;
    UndoButton          _btn_undo;
    RedoButton          _btn_redo;
    EditorModeButton    _btn_editor_mode;
    CloseButton         _btn_close;

    // -----------------------------------------------------------------------
    // Shared UI constants
    // -----------------------------------------------------------------------
    inline static const std::string tab_symbol = { "➡️   " };
    inline static const ftxui::Element spacing = { ftxui::text(" ") };
    inline static const ftxui::Element empty   = ftxui::emptyElement();

    // -----------------------------------------------------------------------
    // Private rendering helpers
    // -----------------------------------------------------------------------

    /// @brief Parse markdown formatting from text and apply FTXUI styles
    struct ParseResult {
        ftxui::Elements elements;
        size_t bytes_consumed;
    };
    ParseResult parse_markdown_segment(const std::string& text, size_t start_pos,
                                       bool is_selected, int cursor_x_in_line);

    /// @brief Render the header bar (updates button states, then assembles hbox)
    ftxui::Element render_header(const std::string& filename, bool modified,
                                 bool can_undo, bool can_redo,
                                 bool bold_active, bool italic_active,
                                 bool underline_active, bool strikethrough_active,
                                 EditorMode editor_mode);

    /// @brief Render the status bar
    ftxui::Element render_status_bar(int cursor_x, int cursor_y,
                                     const std::string& status_message,
                                     bool status_shown, StatusBarType status_type);

    /// @brief Render the shortcuts bar below the writing area
    ftxui::Element render_shortcuts();

    /// @brief Render the text lines with line numbers and selection
    ftxui::Elements render_lines(const std::vector<std::string>& buffer,
                                 int cursor_x, int cursor_y,
                                 int scroll_y, int visible_lines,
                                 std::function<bool(int, int)> is_char_selected_fn,
                                 EditorMode editor_mode);
};
