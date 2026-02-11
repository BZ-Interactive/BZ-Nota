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
