#pragma once
#include <string>
#include <vector>
#include "shared_types.hpp"

/// @brief Manages undo/redo history for the editor
/// UndoRedo pattern,  manual implementation
class UndoRedoManager {
public:
    // Stores complete editor state for undo/redo
    struct EditorState {
        std::vector<std::string> buffer;  // Full text buffer copy
        int cursor_x;                      // Cursor position
        int cursor_y;
    };
    
    UndoRedoManager();
    
    // Save current state to undo history (const& = readonly reference)
    void save_state(
        const std::vector<std::string>& buffer,
        int cursor_x,
        int cursor_y
    );
    
    // Undo - restores previous state, returns success
    // '&' on parameters = modify original values
    bool undo(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );
    
    // Redo - restores next state, returns success
    bool redo(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );
    
    // Inline getters
    bool can_undo() const { return !undo_history.empty(); }
    bool can_redo() const { return !redo_history.empty(); }

private:
    std::vector<EditorState> undo_history;
    std::vector<EditorState> redo_history;
    const static uint8_t max_history = std::numeric_limits<uint8_t>::max();  // 255 is plenty, one may never reach it in practice
};
