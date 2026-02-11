#include "undo_redo_manager.hpp"
#include <algorithm>

UndoRedoManager::UndoRedoManager() {}

// ===== save_state =====
// Called BEFORE each edit (or group of edits).
// If a previous edit is still pending, commit it first.

void UndoRedoManager::save_state(
    const std::vector<std::string>& buffer,
    int cursor_x,
    int cursor_y
) {
    // Commit the previous edit (diff pending snapshot vs current buffer)
    if (has_pending) {
        commit_pending(buffer, cursor_x, cursor_y);
    }

    // Store current buffer as the "before" snapshot for the upcoming edit
    pending_buffer = buffer;
    pending_cx = cursor_x;
    pending_cy = cursor_y;
    has_pending = true;

    // A new edit invalidates the redo history
    redo_stack.clear();
}

// ===== commit_pending =====
// Diffs the pending "before" buffer against the current "after" buffer.
// Only the changed line range is stored as an EditCommand.

void UndoRedoManager::commit_pending(
    const std::vector<std::string>& current_buffer,
    int cursor_x,
    int cursor_y
) {
    if (!has_pending) return;

    const auto& old_buf = pending_buffer;
    const auto& new_buf = current_buffer;
    const int old_size = static_cast<int>(old_buf.size());
    const int new_size = static_cast<int>(new_buf.size());

    // --- Find first differing line from the top ---
    int first_diff = 0;
    while (first_diff < old_size && first_diff < new_size &&
           old_buf[first_diff] == new_buf[first_diff]) {
        first_diff++;
    }

    // --- Find last differing line from the bottom ---
    int old_end = old_size - 1;
    int new_end = new_size - 1;
    while (old_end >= first_diff && new_end >= first_diff &&
           old_buf[old_end] == new_buf[new_end]) {
        old_end--;
        new_end--;
    }

    // If nothing changed, discard
    if (first_diff > old_end && first_diff > new_end) {
        has_pending = false;
        pending_buffer.clear();
        return;
    }

    // --- Build command with only the changed lines ---
    EditCommand cmd;
    cmd.start_line      = first_diff;
    cmd.cursor_x_before = pending_cx;
    cmd.cursor_y_before = pending_cy;
    cmd.cursor_x_after  = cursor_x;
    cmd.cursor_y_after  = cursor_y;

    for (int i = first_diff; i <= old_end; i++) {
        cmd.old_lines.push_back(old_buf[i]);
    }
    for (int i = first_diff; i <= new_end; i++) {
        cmd.new_lines.push_back(new_buf[i]);
    }

    undo_stack.push_back(std::move(cmd));

    // Limit history depth
    if (undo_stack.size() > max_history) {
        undo_stack.erase(undo_stack.begin());
    }

    has_pending = false;
    pending_buffer.clear();
}

// ===== undo =====

bool UndoRedoManager::undo(
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y
) {
    // Commit any in-flight edit first
    if (has_pending) {
        commit_pending(buffer, cursor_x, cursor_y);
    }

    if (undo_stack.empty()) return false;

    EditCommand cmd = std::move(undo_stack.back());
    undo_stack.pop_back();

    // Replace new_lines with old_lines at start_line
    auto it = buffer.begin() + cmd.start_line;
    buffer.erase(it, it + static_cast<int>(cmd.new_lines.size()));
    buffer.insert(buffer.begin() + cmd.start_line,
                  cmd.old_lines.begin(), cmd.old_lines.end());

    cursor_x = cmd.cursor_x_before;
    cursor_y = cmd.cursor_y_before;

    redo_stack.push_back(std::move(cmd));
    return true;
}

// ===== redo =====

bool UndoRedoManager::redo(
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y
) {
    if (redo_stack.empty()) return false;

    EditCommand cmd = std::move(redo_stack.back());
    redo_stack.pop_back();

    // Replace old_lines with new_lines at start_line
    auto it = buffer.begin() + cmd.start_line;
    buffer.erase(it, it + static_cast<int>(cmd.old_lines.size()));
    buffer.insert(buffer.begin() + cmd.start_line,
                  cmd.new_lines.begin(), cmd.new_lines.end());

    cursor_x = cmd.cursor_x_after;
    cursor_y = cmd.cursor_y_after;

    undo_stack.push_back(std::move(cmd));
    return true;
}
