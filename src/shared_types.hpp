#pragma once
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include <functional>

/// @brief Status type used for UI status bars and file operation results
enum class StatusBarType {
    NORMAL,
    SUCCESS,
    ERROR,
    WARNING
};

/// @brief Enum for tracking the last editor action (used by undo grouping)
enum class EditorAction {
    NONE,
    TYPING,
    DELETE,
    DELETE_FORWARD,
    NEWLINE,
    PASTE_SYSTEM,
    UNDO,
    REDO,
    INSERT_LINE,
    TAB,
    UNTAB
};

/// @brief Enum for tracking the current editor mode
enum class EditorMode {
    BASIC, // Basic with no formatting etc. Will show **Bold** as is.
    FANCY, // has formatting like bold/italic, etc. Will show *Italic* with italic tilt.
    CODE, // syntax highlighting, no formatting. Will show **Bold** as is, but keywords in color, for future implementation.
    DOCUMENT // like fancy but with support for dictionary and grammar features, for future implementation.
};

/// @brief Parameters for rendering the editor UI
struct RenderParams {
    const std::vector<std::string>& buffer;
    int cursor_x;
    int cursor_y;
    int scroll_y;
    const std::string& filename;
    bool modified;
    const std::string& status_message;
    bool status_shown;
    StatusBarType status_type;
    EditorMode editor_mode;
    bool can_undo;
    bool can_redo;
    bool bold_active;
    bool italic_active;
    bool underline_active;
    bool strikethrough_active;
    std::function<bool(int, int)> is_char_selected_fn;
};
