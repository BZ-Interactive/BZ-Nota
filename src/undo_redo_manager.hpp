#pragma once
#include <string>
#include <vector>

/// @brief Manages undo/redo history for the editor
class UndoRedoManager {
public:
    struct EditorState {
        std::vector<std::string> buffer;
        int cursor_x;
        int cursor_y;
    };
    
    UndoRedoManager();
    
    void save_state(
        const std::vector<std::string>& buffer,
        int cursor_x,
        int cursor_y
    );
    
    bool undo(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );
    
    bool redo(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );
    
    bool can_undo() const { return !undo_history.empty(); }
    bool can_redo() const { return !redo_history.empty(); }
    
    void set_max_history(int max) { max_history = max; }

private:
    std::vector<EditorState> undo_history;
    std::vector<EditorState> redo_history;
    int max_history = 100;
};
