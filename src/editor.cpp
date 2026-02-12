#include "editor.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdio>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <cstring>
#include <tuple>

using namespace ftxui;

// Global flag for signal handling
// volatile sig_atomic_t = thread-safe atomic type for signal handlers
static volatile sig_atomic_t ctrl_c_pressed = 0;

static void handle_sigint(int sig) {
    (void)sig;  // Unused parameter (suppress warning)
    ctrl_c_pressed = 1;
}

// ===== Constructor / Destructor =====
// Constructor initializer list (more efficient than assigning in body)
Editor::Editor(const std::string& fn, bool dbg) 
    : filename(fn), debug_mode(dbg) {
    load_file();
}

// Destructor - automatically called when object goes out of scope (RAII)
// Note to self: Unlike C# where GC handles cleanup, C++ destructors are deterministic
Editor::~Editor() {
    if (modified) {
        std::cerr << "Warning: Unsaved changes!" << std::endl;
    }
}

bool Editor::set_editor_mode(EditorMode mode) {
    if (mode == editor_mode) return false; // No change
    
    if (mode == EditorMode::CODE || mode == EditorMode::DOCUMENT) {
        // check for supported languages ONLY APPLICABLE AFTER SYNTAX HIGHLIGHTING IS IMPLEMENTED
        
        // temporary
        if (editor_mode == EditorMode::FANCY) {
            editor_mode = EditorMode::BASIC;
            set_status("Switched editor mode to BASIC");
        }
        else if (editor_mode == EditorMode::BASIC) {
            editor_mode = EditorMode::FANCY;
            set_status("Switched editor mode to FANCY");
        }
        return true;
        // Temporary end

        #if 0
        set_status("This mode is not implemented yet", StatusBarType::WARNING);
        return false;
        #endif

        // Check if supported coding language via file extension
        // if so return true;

        // check if supported language in document
        // if so return true;
    }

    editor_mode = mode;
    set_status("Switched editor mode to " + std::to_string(static_cast<int>(mode)));
    return true;
}

// ===== File Operations =====

void Editor::load_file() {
    FileOperationResult result = file_manager.load_file(filename, buffer);
    if(!result.success) {
        set_status(result.message, result.status_type);
    }
}

void Editor::save_file() {
    FileOperationResult result = file_manager.save_file(filename, buffer);

    set_status(result.message, result.status_type);
    if (result.success)
        modified = false;
}

void Editor::rename_file(const std::string& new_filename) {
    // If the current file doesn't exist on disk yet (unsaved/new file),
    // just adopt the new name and save directly — there's nothing to rename.
    std::ifstream source_check(filename);
    bool source_exists = source_check.good();
    source_check.close();

    if (!source_exists) {
        filename = new_filename;
        save_file();
        return;
    }

    FileOperationResult result = file_manager.rename_file(filename, new_filename);
    
    set_status(result.message, result.status_type);
    if (result.success) {
        filename = new_filename;
    }
}

// ===== Selection Operations =====

void Editor::start_selection() {
    selection_manager.start_selection(cursor_x, cursor_y);
}

void Editor::update_selection() {
    selection_manager.update_selection(cursor_x, cursor_y);
}

void Editor::clear_selection() {
    selection_manager.clear_selection();
}

void Editor::delete_selection() {
    if (!selection_manager.has_active_selection()) return;
    
    save_state();
    selection_manager.delete_selection(buffer, cursor_x, cursor_y);
    clamp_cursor_and_scroll();
    modified = true;
}

void Editor::select_all() {
    if (buffer.empty()) return;
    
    int end_y = buffer.size() - 1;
    int end_x = buffer[end_y].size();
    selection_manager.select_all(end_x, end_y);
    set_status("Selected all");
}

std::string Editor::get_selected_text() const {
    return selection_manager.get_selected_text(buffer);
}

bool Editor::is_char_selected(int x, int y) const {
    return selection_manager.is_char_selected(x, y);
}

// ===== Clipboard Operations =====

void Editor::copy_to_system_clipboard() {
    const std::string text = get_selected_text();
    if (text.empty()) {
        set_status("No text selected");
        return;
    }
    
    if (clipboard_manager.copy_to_system(text)) {
        set_status("Copied " + std::to_string(text.length()) + " chars to system clipboard");
    } else {
        set_status("Failed to copy to system clipboard (check xclip/wl-clipboard)");
    }
}

void Editor::paste_from_system_clipboard() {
    save_state();
    typing_state_saved = false;
    last_action = EditorAction::PASTE_SYSTEM;
    
    if (selection_manager.has_active_selection()) {
        delete_selection();
    }
    
    int char_count = clipboard_manager.paste_from_system(buffer, cursor_x, cursor_y);
    
    if (char_count < 0) {
        set_status("Failed to paste from system clipboard");
    } else if (char_count == 0) {
        set_status("System clipboard is empty");
    } else {
        set_status("Pasted " + std::to_string(char_count) + " characters from system clipboard");
        modified = true;
    }
}

void Editor::cut_to_system_clipboard() {
    const std::string text = get_selected_text();
    if (text.empty()) {
        set_status("No text selected");
        return;
    }
    
    if (clipboard_manager.copy_to_system(text)) {
        delete_selection();
        set_status("Cut " + std::to_string(text.length()) + " chars to system clipboard");
        modified = true;
    } else {
        set_status("Failed to cut to system clipboard (check xclip/wl-clipboard)");
    }
}

// ===== Formatting Operations =====

void Editor::toggle_bold() { toggle_format(FormatType::BOLD); }

void Editor::toggle_italic() { toggle_format(FormatType::ITALIC); }

void Editor::toggle_underline() { toggle_format(FormatType::UNDERLINE); }

void Editor::toggle_strikethrough() { toggle_format(FormatType::STRIKETHROUGH); }

/// @brief Helper to get a human-readable name for a FormatType
static const char* format_type_name(FormatType ft) {
    switch (ft) {
        case FormatType::BOLD:          return "Bold";
        case FormatType::ITALIC:        return "Italic";
        case FormatType::UNDERLINE:     return "Underline";
        case FormatType::STRIKETHROUGH: return "Strikethrough";
    }
    return "";
}

void Editor::toggle_format(FormatType format_type) {
    // If there's an active selection, wrap/unwrap it with markers
    if (selection_manager.has_active_selection()) {
        selection_manager.adjust_selection_for_formatting(buffer);
        std::string selected_text = get_selected_text();
        if (!selected_text.empty()) {
            bool has_bold, has_italic, has_underline, has_strikethrough;
            std::string plain_text = format_manager.extract_formatting_from_text(
                selected_text, has_bold, has_italic, has_underline, has_strikethrough);

            delete_selection();

            std::string rebuilt = plain_text;
            // Preserve other formatting
            if (format_type != FormatType::BOLD && has_bold) rebuilt = format_manager.wrap_with_bold(rebuilt);
            if (format_type != FormatType::ITALIC && has_italic) rebuilt = format_manager.wrap_with_italic(rebuilt);
            if (format_type != FormatType::UNDERLINE && has_underline) rebuilt = format_manager.wrap_with_underline(rebuilt);
            if (format_type != FormatType::STRIKETHROUGH && has_strikethrough) rebuilt = format_manager.wrap_with_strikethrough(rebuilt);

            bool had = false;
            switch (format_type) {
                case FormatType::BOLD:          had = has_bold; break;
                case FormatType::ITALIC:        had = has_italic; break;
                case FormatType::UNDERLINE:     had = has_underline; break;
                case FormatType::STRIKETHROUGH: had = has_strikethrough; break;
            }

            // Toggle behavior: apply format only if it wasn't present
            if (!had) {
                switch (format_type) {
                    case FormatType::BOLD:          rebuilt = format_manager.wrap_with_bold(rebuilt); break;
                    case FormatType::ITALIC:        rebuilt = format_manager.wrap_with_italic(rebuilt); break;
                    case FormatType::UNDERLINE:     rebuilt = format_manager.wrap_with_underline(rebuilt); break;
                    case FormatType::STRIKETHROUGH: rebuilt = format_manager.wrap_with_strikethrough(rebuilt); break;
                }
            }

            editing_manager.insert_string(buffer, cursor_x, cursor_y, rebuilt);
            modified = true;
            std::string name = format_type_name(format_type);
            set_status(had ? (name + " formatting removed") : (name + " formatting applied to selection"));
            return;
        }
    }

    // No selection: act on formatting at cursor
    bool bold_at_cursor, italic_at_cursor, underline_at_cursor, strikethrough_at_cursor;
    cursor_manager.get_formatting_at_cursor(buffer[cursor_y], cursor_x,
                                           bold_at_cursor, italic_at_cursor,
                                           underline_at_cursor, strikethrough_at_cursor);

    bool is_active = false;
    switch (format_type) {
        case FormatType::BOLD:          is_active = bold_at_cursor; break;
        case FormatType::ITALIC:        is_active = italic_at_cursor; break;
        case FormatType::UNDERLINE:     is_active = underline_at_cursor; break;
        case FormatType::STRIKETHROUGH: is_active = strikethrough_at_cursor; break;
    }

    if (is_active) {
        const std::string& line = buffer[cursor_y];
        size_t closing_pos = std::string::npos;
        size_t marker_len = 0;
        switch (format_type) {
            case FormatType::BOLD:          marker_len = 2; closing_pos = line.find("**", cursor_x); break;
            case FormatType::ITALIC:        marker_len = 1; closing_pos = line.find("*", cursor_x); break;
            case FormatType::UNDERLINE:     marker_len = 4; closing_pos = line.find("</u>", cursor_x); break;
            case FormatType::STRIKETHROUGH: marker_len = 2; closing_pos = line.find("~~", cursor_x); break;
        }

        if (closing_pos == (size_t)cursor_x) {
            cursor_x += marker_len;
            switch (format_type) {
                case FormatType::BOLD:          format_manager.toggle_bold(); break;
                case FormatType::ITALIC:        format_manager.toggle_italic(); break;
                case FormatType::UNDERLINE:     format_manager.toggle_underline(); break;
                case FormatType::STRIKETHROUGH: format_manager.toggle_strikethrough(); break;
            }
            set_status(format_manager.get_status_message());
            return;
        }

        // Cursor is in middle - split formatting
        format_manager.split_formatting_at_cursor(buffer, cursor_x, cursor_y, format_type);
        modified = true;
        set_status(std::string(format_type_name(format_type)) + " formatting split");
        return;
    }

    // Otherwise, enable formatting session globally
    switch (format_type) {
        case FormatType::BOLD:          format_manager.toggle_bold(); break;
        case FormatType::ITALIC:        format_manager.toggle_italic(); break;
        case FormatType::UNDERLINE:     format_manager.toggle_underline(); break;
        case FormatType::STRIKETHROUGH: format_manager.toggle_strikethrough(); break;
    }
    set_status(format_manager.get_status_message());
}

// Deprecated duplicate implementation removed - now handled by toggle_format("italic");

// Deprecated duplicate implementation removed - now handled by toggle_format("underline");

// Deprecated duplicate implementation removed - now handled by toggle_format("strikethrough");

// ===== Editing Operations =====

void Editor::insert_char(char c) {
    delete_selection_if_active();
    
    // Check if we're inside existing formatting markers
    bool inside_markers = cursor_manager.is_cursor_inside_formatting_markers(buffer[cursor_y], cursor_x);
    
    // Insert formatting markers if active and not already inside formatted text
    if (format_manager.has_active_formatting() && !inside_markers) {
        // Insert both opening and closing markers, cursor stays between them
        format_manager.insert_formatting_markers(buffer, cursor_x, cursor_y);
        modified = true;
    }
    
    editing_manager.insert_char(buffer, cursor_x, cursor_y, c);
    modified = true;
}

void Editor::insert_string(const std::string& str) {
    delete_selection_if_active();
    
    // Check if we're inside existing formatting markers
    bool inside_markers = cursor_manager.is_cursor_inside_formatting_markers(buffer[cursor_y], cursor_x);
    
    // Insert formatting markers if active and not already inside formatted text
    if (format_manager.has_active_formatting() && !inside_markers) {
        // Insert both opening and closing markers, cursor stays between them
        format_manager.insert_formatting_markers(buffer, cursor_x, cursor_y);
        modified = true;
    }
    
    editing_manager.insert_string(buffer, cursor_x, cursor_y, str);
    modified = true;
}

void Editor::insert_newline() {
    delete_selection_if_active();
    
    save_state();
    typing_state_saved = false;
    last_action = EditorAction::NEWLINE;
    editing_manager.insert_newline(buffer, cursor_x, cursor_y);
    modified = true;
}

// --- New helper methods to encapsulate input-related buffer edits ---
void Editor::insert_line_above() {
    delete_selection_if_active();
    save_state();
    typing_state_saved = false;
    last_action = EditorAction::INSERT_LINE;
    buffer.insert(buffer.begin() + cursor_y, "");
    cursor_x = 0;
    modified = true;
}

void Editor::insert_line_below() {
    delete_selection_if_active();
    save_state();
    typing_state_saved = false;
    last_action = EditorAction::INSERT_LINE;
    buffer.insert(buffer.begin() + cursor_y + 1, "");
    cursor_y++;
    cursor_x = 0;
    modified = true;
}

void Editor::insert_tab() {
    delete_selection_if_active();
    save_state();
    typing_state_saved = false;
    last_action = EditorAction::TAB;
    buffer[cursor_y].insert(cursor_x, "\t");
    cursor_x++;
    modified = true;
}

void Editor::unindent_current_line() {
    delete_selection_if_active();
    if (!buffer[cursor_y].empty() && buffer[cursor_y][0] == '\t') {
        save_state();
        typing_state_saved = false;
        last_action = EditorAction::UNTAB;
        buffer[cursor_y].erase(0, 1);
        if (cursor_x > 0) cursor_x--;
        modified = true;
    }
}

void Editor::reset_status() {
    status_shown = false;
    status_bar_type = StatusBarType::NORMAL;
}

void Editor::delete_char() {
    if (selection_manager.has_active_selection()) {
        delete_selection();
        return;
    }
    
    if (last_action != EditorAction::DELETE) {
        save_state();
        last_action = EditorAction::DELETE;
    }
    typing_state_saved = false;
    
    editing_manager.delete_char(buffer, cursor_x, cursor_y);
    clamp_cursor_and_scroll();
    modified = true;
}

void Editor::delete_forward() {
    if (selection_manager.has_active_selection()) {
        delete_selection();
        return;
    }
    
    if (last_action != EditorAction::DELETE_FORWARD) {
        save_state();
        last_action = EditorAction::DELETE_FORWARD;
    }
    typing_state_saved = false;
    
    editing_manager.delete_forward(buffer, cursor_x, cursor_y);
    clamp_cursor_and_scroll();
    modified = true;
}

// ===== Cursor Movement =====

void Editor::move_cursor_left(bool select) {
    auto [update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_selection();
    cursor_manager.move_left(buffer, cursor_x, cursor_y, update_sel, clear_sel, select);
}

void Editor::move_cursor_right(bool select) {
    auto [update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_selection();
    cursor_manager.move_right(buffer, cursor_x, cursor_y, update_sel, clear_sel, select);
}

void Editor::move_cursor_up(bool select) {
    auto [update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_selection();
    cursor_manager.move_up(buffer, cursor_x, cursor_y, update_sel, clear_sel, select);
}

void Editor::move_cursor_down(bool select) {
    auto [update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_selection();
    cursor_manager.move_down(buffer, cursor_x, cursor_y, update_sel, clear_sel, select);
}

void Editor::move_word_left(bool select) {
    auto [update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_selection();
    cursor_manager.move_word_left(buffer, cursor_x, cursor_y, update_sel, clear_sel, select);
}

void Editor::move_word_right(bool select) {
    auto [update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_selection();
    cursor_manager.move_word_right(buffer, cursor_x, cursor_y, update_sel, clear_sel, select);
}

void Editor::move_cursor_home(bool select) {
    if (select && !selection_manager.has_active_selection()) start_selection();
    auto [update_sel, clear_sel] = get_selection_callbacks();
    cursor_manager.move_home(buffer, cursor_x, cursor_y, update_sel, clear_sel, select);
}

void Editor::move_cursor_end(bool select) {
    if (select && !selection_manager.has_active_selection()) start_selection();
    auto [update_sel, clear_sel] = get_selection_callbacks();
    cursor_manager.move_end(buffer, cursor_x, cursor_y, update_sel, clear_sel, select);
}

// ===== Helper Functions =====

int Editor::find_word_start(int x, int y) {
    return cursor_manager.find_word_start(buffer[y], x);
}

int Editor::find_word_end(int x, int y) {
    return cursor_manager.find_word_end(buffer[y], x);
}

void Editor::ensure_cursor_visible(int screen_height) {
    cursor_manager.ensure_cursor_visible(cursor_y, scroll_y, screen_height);
}

void Editor::clamp_cursor_and_scroll() {
    // Ensure buffer is never empty
    if (buffer.empty()) buffer.push_back("");
    
    // Clamp cursor_y
    if (cursor_y < 0) cursor_y = 0;
    if (cursor_y >= (int)buffer.size()) cursor_y = (int)buffer.size() - 1;
    
    // Clamp cursor_x
    if (cursor_x < 0) cursor_x = 0;
    if (cursor_x > (int)buffer[cursor_y].length()) cursor_x = (int)buffer[cursor_y].length();
    
    // Clamp scroll_y
    if (scroll_y < 0) scroll_y = 0;
    if (scroll_y >= (int)buffer.size()) scroll_y = std::max(0, (int)buffer.size() - 1);
}

void Editor::set_status(const std::string& message, StatusBarType type) {
    status_bar_type = type;
    status_message = message;
    status_shown = true;
}

void Editor::delete_selection_if_active() {
    if (selection_manager.has_active_selection()) {
        delete_selection();
    }
}

std::tuple<std::function<void()>, std::function<void()>> Editor::get_selection_callbacks() {
    auto update_sel = [this]() { update_selection(); };
    auto clear_sel = [this]() { clear_selection(); };
    return std::make_tuple(update_sel, clear_sel);
}

// ===== Undo/Redo =====

void Editor::save_state() {
    undo_redo_manager.save_state(buffer, cursor_x, cursor_y);
}

void Editor::undo() {
    if (!undo_redo_manager.can_undo()) {
        set_status("Nothing to undo");
        return;
    }
    
    typing_state_saved = false;
    last_action = EditorAction::UNDO;
    undo_redo_manager.undo(buffer, cursor_x, cursor_y);
    clamp_cursor_and_scroll();
    modified = true;
    set_status("Undo");
}

void Editor::redo() {
    if (!undo_redo_manager.can_redo()) {
        set_status("Nothing to redo");
        return;
    }
    
    typing_state_saved = false;
    last_action = EditorAction::REDO;
    undo_redo_manager.redo(buffer, cursor_x, cursor_y);
    clamp_cursor_and_scroll();
    modified = true;
    set_status("Redo");
}

// ===== UI Rendering =====

Element Editor::render() {
    clamp_cursor_and_scroll();
    int screen_height = Terminal::Size().dimy;
    ensure_cursor_visible(screen_height);
    
    // Check if cursor is inside formatting markers
    bool bold_at_cursor, italic_at_cursor, underline_at_cursor, strikethrough_at_cursor;
    cursor_manager.get_formatting_at_cursor(buffer[cursor_y], cursor_x, 
                                           bold_at_cursor, italic_at_cursor, 
                                           underline_at_cursor, strikethrough_at_cursor);
    
    // Show formatting as active if either globally active or cursor is inside formatted text
    bool show_bold = format_manager.is_bold() || bold_at_cursor;
    bool show_italic = format_manager.is_italic() || italic_at_cursor;
    bool show_underline = format_manager.is_underline() || underline_at_cursor;
    bool show_strikethrough = format_manager.is_strikethrough() || strikethrough_at_cursor;
    
    // Use UIRenderer to handle all rendering
    auto is_char_selected_fn = [this](int x, int y) {
        return this->is_char_selected(x, y);
    };
    
    RenderParams params{
        buffer,
        cursor_x, cursor_y,
        scroll_y,
        filename,
        modified,
        status_message,
        status_shown,
        status_bar_type,
        editor_mode,
        undo_redo_manager.can_undo(),
        undo_redo_manager.can_redo(),
        show_bold,
        show_italic,
        show_underline,
        show_strikethrough,
        is_char_selected_fn
    };
    
    return ui_renderer.render(params);
}

// ===== Event Handling =====

bool Editor::handle_event(Event event) {
    return input_manager.handle_event(event, *this, ctrl_c_pressed);
}

/// @brief Clear the UI and redraw, if the file isn't modified reload it from disk.
void Editor::screen_reset() {
    screen->Clear();
    if (!is_modified())
        load_file();
    set_status("UI Reset.", StatusBarType::NORMAL);
}

void Editor::exit() {
    screen->Exit();
}

// ===== Main Loop =====

void Editor::run() {
    // Setup signal handling
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    
    // Ignore other signals
    signal(SIGTSTP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    
    // Set terminal to raw mode with signals disabled
    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ISIG); // Disable signal generation
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    
    // Create FTXUI screen
    auto screen_instance = ScreenInteractive::FullscreenPrimaryScreen();
    screen = &screen_instance;
    
    // Force FTXUI to not handle Ctrl+C automatically
    screen->ForceHandleCtrlC(false);
    
    // Disable mouse support
    screen->TrackMouse(false);
    
    // Create component
    auto component = Renderer([&] {
        return render();
    });
    
    component = CatchEvent(component, [&](Event event) {
        return handle_event(event);
    });
    
    // Run main loop
    screen->Loop(component);
    
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}
