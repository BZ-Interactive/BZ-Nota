#include "undo_redo_manager.hpp"

UndoRedoManager::UndoRedoManager() {}

void UndoRedoManager::save_state(
    const std::vector<std::string>& buffer,
    int cursor_x,
    int cursor_y
) {
    EditorState state;
    state.buffer = buffer;
    state.cursor_x = cursor_x;
    state.cursor_y = cursor_y;
    
    undo_history.push_back(state);
    
    // Limit history size
    if (undo_history.size() > (size_t)max_history) {
        undo_history.erase(undo_history.begin());
    }
    
    // Clear redo history when new action is performed
    redo_history.clear();
}

bool UndoRedoManager::undo(
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y
) {
    if (undo_history.empty()) {
        return false;
    }
    
    // Save current state to redo history
    EditorState current_state;
    current_state.buffer = buffer;
    current_state.cursor_x = cursor_x;
    current_state.cursor_y = cursor_y;
    redo_history.push_back(current_state);
    
    // Restore previous state
    EditorState prev_state = undo_history.back();
    undo_history.pop_back();
    
    buffer = prev_state.buffer;
    cursor_x = prev_state.cursor_x;
    cursor_y = prev_state.cursor_y;
    
    return true;
}

bool UndoRedoManager::redo(
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y
) {
    if (redo_history.empty()) {
        return false;
    }
    
    // Save current state to undo history
    EditorState current_state;
    current_state.buffer = buffer;
    current_state.cursor_x = cursor_x;
    current_state.cursor_y = cursor_y;
    undo_history.push_back(current_state);
    
    // Restore redo state
    EditorState next_state = redo_history.back();
    redo_history.pop_back();
    
    buffer = next_state.buffer;
    cursor_x = next_state.cursor_x;
    cursor_y = next_state.cursor_y;
    
    return true;
}
