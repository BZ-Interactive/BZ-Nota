#pragma once
#include <cstdint>
#include <limits>

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
