#pragma once
#include <string>
#include <vector>

/// @brief Manages undo/redo history using the Command pattern.
///
/// Instead of storing full buffer snapshots on every edit, this stores
/// only the lines that changed (a diff). Memory usage goes from
/// O(history_depth * buffer_size) to O(buffer_size + sum_of_diffs).
///
/// The public API (save_state, undo, redo) is unchanged from the old
/// snapshot-based approach, so callers don't need any modifications.
class UndoRedoManager {
public:
    /// @brief Represents a single edit as a diff of the affected line range.
    ///
    /// To undo: replace new_lines with old_lines at start_line.
    /// To redo: replace old_lines with new_lines at start_line.
    struct EditCommand {
        int start_line;                      // First line in the affected range
        std::vector<std::string> old_lines;  // Lines *before* the edit
        std::vector<std::string> new_lines;  // Lines *after*  the edit
        int cursor_x_before, cursor_y_before;
        int cursor_x_after,  cursor_y_after;
    };

    UndoRedoManager();

    /// @brief Call before an edit begins. Captures a "before" snapshot.
    ///
    /// If a previous edit was still pending (not yet committed), this
    /// commits it first by diffing the pending snapshot against the
    /// current buffer, then stores that diff as an EditCommand.
    void save_state(
        const std::vector<std::string>& buffer,
        int cursor_x,
        int cursor_y
    );

    /// @brief Undo the most recent edit.
    bool undo(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );

    /// @brief Redo the most recently undone edit.
    bool redo(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );

    bool can_undo() const { return has_pending || !undo_stack.empty(); }
    bool can_redo() const { return !redo_stack.empty(); }

private:
    /// @brief Diff pending_buffer vs current buffer, push result to undo_stack.
    void commit_pending(
        const std::vector<std::string>& current_buffer,
        int cursor_x,
        int cursor_y
    );

    // --- Pending edit tracking ---
    bool has_pending = false;
    std::vector<std::string> pending_buffer;  // Temporary "before" snapshot
    int pending_cx = 0;
    int pending_cy = 0;

    // --- Command stacks ---
    std::vector<EditCommand> undo_stack;
    std::vector<EditCommand> redo_stack;

    static constexpr size_t max_history = 255;
};
