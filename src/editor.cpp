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

// ===== File Operations =====

void Editor::load_file() {
    std::ifstream ifs(filename);
    if (!ifs) {
        buffer.push_back("");
        return;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        buffer.push_back(line);
    }
    if (buffer.empty()) {
        buffer.push_back("");
    }
}

void Editor::save_file() {
    // Create directory if needed
    char* filename_copy = strdup(filename.c_str());
    char* dir = dirname(filename_copy);
    mkdir(dir, 0755);
    free(filename_copy);
    
    std::ofstream ofs(filename);
    if (!ofs) {
        int err = errno;
        if (err == EACCES) {
            set_status("Error: Permission denied when saving file!, Save as sudo? (y/n)", UIRenderer::StatusBarType::ERROR);
            // TODO: implement "save as sudo" flow - either pipe file or get permissions here with elevated permissions
        } else if (err == ENOENT) {
            set_status("Error: Directory does not exist!", UIRenderer::StatusBarType::ERROR);
        } else  if(err == EAGAIN) {
            set_status("Error: Try again!", UIRenderer::StatusBarType::WARNING);
        } else if (err == EROFS) {
            set_status("Error: Read-only file system!", UIRenderer::StatusBarType::ERROR);
        } else {
            set_status("Error: Could not save file! (" + std::string(strerror(err)) + ")", UIRenderer::StatusBarType::ERROR);
        }
        return;
    }
    // this basically means the IO is corrupt, good luck.
    else if (ofs.bad()) {
        set_status("Error: I/O error while saving file!", UIRenderer::StatusBarType::ERROR);
        return;
    }
    
    for (const auto& line : buffer) {
        ofs << line << '\n';
    }
    
    modified = false;
    set_status("File saved successfully", UIRenderer::StatusBarType::SUCCESS);
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
    modified = true;
}

void Editor::select_all() {
    if (buffer.empty()) return;
    
    int end_y = buffer.size() - 1;
    int end_x = buffer[end_y].size();
    selection_manager.select_all(end_x, end_y);
    set_status("Selected all");
}

std::string Editor::get_selected_text() {
    return selection_manager.get_selected_text(buffer);
}

bool Editor::is_char_selected(int x, int y) {
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

void Editor::toggle_bold() {
    // If there's an active selection, wrap it with bold markers
    if (selection_manager.has_active_selection()) {
        // Adjust selection to include opening formatting markers
        selection_manager.adjust_selection_for_formatting(buffer);
        
        std::string selected_text = get_selected_text();
        if (!selected_text.empty()) {
            // Extract existing formatting from selected text
            bool has_bold, has_italic, has_underline, has_strikethrough;
            std::string plain_text = format_manager.extract_formatting_from_text(
                selected_text, has_bold, has_italic, has_underline, has_strikethrough);
            
            delete_selection();
            
            // Rebuild: wrap plain text with existing formatting, excluding bold
            std::string rebuilt = plain_text;
            if (has_italic) rebuilt = format_manager.wrap_with_italic(rebuilt);
            if (has_underline) rebuilt = format_manager.wrap_with_underline(rebuilt);
            if (has_strikethrough) rebuilt = format_manager.wrap_with_strikethrough(rebuilt);
            
            // Only apply bold if it wasn't already present (toggle behavior)
            if (!has_bold) {
                rebuilt = format_manager.wrap_with_bold(rebuilt);
            }
            
            editing_manager.insert_string(buffer, cursor_x, cursor_y, rebuilt);
            modified = true;
            set_status(has_bold ? "Bold formatting removed" : "Bold formatting applied to selection");
            return;
        }
    }
    
    // Check if cursor is inside bold markers
    bool bold_at_cursor, italic_at_cursor, underline_at_cursor, strikethrough_at_cursor;
    cursor_manager.get_formatting_at_cursor(buffer[cursor_y], cursor_x, 
                                           bold_at_cursor, italic_at_cursor, 
                                           underline_at_cursor, strikethrough_at_cursor);
    
    // If cursor is inside bold markers
    if (bold_at_cursor) {
        // Check if cursor is right at the closing marker (end of formatted text)
        const std::string& line = buffer[cursor_y];
        size_t closing = line.find("**", cursor_x);
        if (closing == (size_t)cursor_x) {
            // Just move cursor past the closing marker and disable bold
            cursor_x += 2;
            format_manager.toggle_bold();
            set_status(format_manager.get_status_message());
            return;
        }
        
        // Cursor is in the middle - split the formatting
        format_manager.split_formatting_at_cursor(buffer, cursor_x, cursor_y, "bold");
        modified = true;
        set_status("Bold formatting split");
        return;
    }
    
    format_manager.toggle_bold();
    set_status(format_manager.get_status_message());
}

void Editor::toggle_italic() {
    // If there's an active selection, wrap it with italic markers
    if (selection_manager.has_active_selection()) {
        // Adjust selection to include opening formatting markers
        selection_manager.adjust_selection_for_formatting(buffer);
        
        std::string selected_text = get_selected_text();
        if (!selected_text.empty()) {
            // Extract existing formatting from selected text
            bool has_bold, has_italic, has_underline, has_strikethrough;
            std::string plain_text = format_manager.extract_formatting_from_text(
                selected_text, has_bold, has_italic, has_underline, has_strikethrough);
            
            delete_selection();
            
            // Rebuild: wrap plain text with existing formatting, excluding italic
            std::string rebuilt = plain_text;
            if (has_bold) rebuilt = format_manager.wrap_with_bold(rebuilt);
            if (has_underline) rebuilt = format_manager.wrap_with_underline(rebuilt);
            if (has_strikethrough) rebuilt = format_manager.wrap_with_strikethrough(rebuilt);
            
            // Only apply italic if it wasn't already present (toggle behavior)
            if (!has_italic) {
                rebuilt = format_manager.wrap_with_italic(rebuilt);
            }
            
            editing_manager.insert_string(buffer, cursor_x, cursor_y, rebuilt);
            modified = true;
            set_status(has_italic ? "Italic formatting removed" : "Italic formatting applied to selection");
            return;
        }
    }
    
    // Check if cursor is inside formatting markers
    bool bold_at_cursor, italic_at_cursor, underline_at_cursor, strikethrough_at_cursor;
    cursor_manager.get_formatting_at_cursor(buffer[cursor_y], cursor_x, 
                                           bold_at_cursor, italic_at_cursor, 
                                           underline_at_cursor, strikethrough_at_cursor);
    
    // If cursor is inside italic markers
    if (italic_at_cursor) {
        const std::string& line = buffer[cursor_y];
        size_t closing = line.find("*", cursor_x);
        // Make sure it's not part of **
        if (closing == (size_t)cursor_x && 
            (closing == 0 || line[closing - 1] != '*') && 
            (closing + 1 >= line.length() || line[closing + 1] != '*')) {
            cursor_x += 1;
            format_manager.toggle_italic();
            set_status(format_manager.get_status_message());
            return;
        }
        
        format_manager.split_formatting_at_cursor(buffer, cursor_x, cursor_y, "italic");
        modified = true;
        set_status("Italic formatting split");
        return;
    }
    
    format_manager.toggle_italic();
    set_status(format_manager.get_status_message());
}

void Editor::toggle_underline() {
    // If there's an active selection, wrap it with underline markers
    if (selection_manager.has_active_selection()) {
        // Adjust selection to include opening formatting markers
        selection_manager.adjust_selection_for_formatting(buffer);
        
        std::string selected_text = get_selected_text();
        if (!selected_text.empty()) {
            // Extract existing formatting from selected text
            bool has_bold, has_italic, has_underline, has_strikethrough;
            std::string plain_text = format_manager.extract_formatting_from_text(
                selected_text, has_bold, has_italic, has_underline, has_strikethrough);
            
            delete_selection();
            
            // Rebuild: wrap plain text with existing formatting, excluding underline
            std::string rebuilt = plain_text;
            if (has_italic) rebuilt = format_manager.wrap_with_italic(rebuilt);
            if (has_bold) rebuilt = format_manager.wrap_with_bold(rebuilt);
            if (has_strikethrough) rebuilt = format_manager.wrap_with_strikethrough(rebuilt);
            
            // Only apply underline if it wasn't already present (toggle behavior)
            if (!has_underline) {
                rebuilt = format_manager.wrap_with_underline(rebuilt);
            }
            
            editing_manager.insert_string(buffer, cursor_x, cursor_y, rebuilt);
            modified = true;
            set_status(has_underline ? "Underline formatting removed" : "Underline formatting applied to selection");
            return;
        }
    }
    
    // Check if cursor is inside formatting markers
    bool bold_at_cursor, italic_at_cursor, underline_at_cursor, strikethrough_at_cursor;
    cursor_manager.get_formatting_at_cursor(buffer[cursor_y], cursor_x, 
                                           bold_at_cursor, italic_at_cursor, 
                                           underline_at_cursor, strikethrough_at_cursor);
    
    // If cursor is inside underline markers
    if (underline_at_cursor) {
        const std::string& line = buffer[cursor_y];
        size_t closing = line.find("</u>", cursor_x);
        if (closing == (size_t)cursor_x) {
            cursor_x += 4;
            format_manager.toggle_underline();
            set_status(format_manager.get_status_message());
            return;
        }
        
        format_manager.split_formatting_at_cursor(buffer, cursor_x, cursor_y, "underline");
        modified = true;
        set_status("Underline formatting split");
        return;
    }
    
    format_manager.toggle_underline();
    set_status(format_manager.get_status_message());
}

void Editor::toggle_strikethrough() {
    // If there's an active selection, wrap it with strikethrough markers
    if (selection_manager.has_active_selection()) {
        // Adjust selection to include opening formatting markers
        selection_manager.adjust_selection_for_formatting(buffer);
        
        std::string selected_text = get_selected_text();
        if (!selected_text.empty()) {
            // Extract existing formatting from selected text
            bool has_bold, has_italic, has_underline, has_strikethrough;
            std::string plain_text = format_manager.extract_formatting_from_text(
                selected_text, has_bold, has_italic, has_underline, has_strikethrough);
            
            delete_selection();
            
            // Rebuild: wrap plain text with existing formatting, excluding strikethrough
            std::string rebuilt = plain_text;
            if (has_italic) rebuilt = format_manager.wrap_with_italic(rebuilt);
            if (has_bold) rebuilt = format_manager.wrap_with_bold(rebuilt);
            if (has_underline) rebuilt = format_manager.wrap_with_underline(rebuilt);
            
            // Only apply strikethrough if it wasn't already present (toggle behavior)
            if (!has_strikethrough) {
                rebuilt = format_manager.wrap_with_strikethrough(rebuilt);
            }
            
            editing_manager.insert_string(buffer, cursor_x, cursor_y, rebuilt);
            modified = true;
            set_status(has_strikethrough ? "Strikethrough formatting removed" : "Strikethrough formatting applied to selection");
            return;
        }
    }
    
    // Check if cursor is inside formatting markers
    bool bold_at_cursor, italic_at_cursor, underline_at_cursor, strikethrough_at_cursor;
    cursor_manager.get_formatting_at_cursor(buffer[cursor_y], cursor_x, 
                                           bold_at_cursor, italic_at_cursor, 
                                           underline_at_cursor, strikethrough_at_cursor);
    
    // If cursor is inside strikethrough markers
    if (strikethrough_at_cursor) {
        const std::string& line = buffer[cursor_y];
        size_t closing = line.find("~~", cursor_x);
        if (closing == (size_t)cursor_x) {
            cursor_x += 2;
            format_manager.toggle_strikethrough();
            set_status(format_manager.get_status_message());
            return;
        }
        
        format_manager.split_formatting_at_cursor(buffer, cursor_x, cursor_y, "strikethrough");
        modified = true;
        set_status("Strikethrough formatting split");
        return;
    }
    
    format_manager.toggle_strikethrough();
    set_status(format_manager.get_status_message());
}

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
    modified = true;
}

// ===== Cursor Movement =====

void Editor::move_cursor_left(bool select) {
    auto [start_sel, update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_sel();
    cursor_manager.move_left(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_cursor_right(bool select) {
    auto [start_sel, update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_sel();
    cursor_manager.move_right(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_cursor_up(bool select) {
    auto [start_sel, update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_sel();
    cursor_manager.move_up(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_cursor_down(bool select) {
    auto [start_sel, update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_sel();
    cursor_manager.move_down(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_word_left(bool select) {
    auto [start_sel, update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_sel();
    cursor_manager.move_word_left(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_word_right(bool select) {
    auto [start_sel, update_sel, clear_sel] = get_selection_callbacks();
    if (select && !selection_manager.has_active_selection()) start_sel();
    cursor_manager.move_word_right(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_cursor_home(bool select) {
    if (select && !selection_manager.has_active_selection()) start_selection();
    auto [_, update_sel, clear_sel] = get_selection_callbacks();
    cursor_manager.move_home(buffer, cursor_x, cursor_y, update_sel, clear_sel, select);
}

void Editor::move_cursor_end(bool select) {
    if (select && !selection_manager.has_active_selection()) start_selection();
    auto [_, update_sel, clear_sel] = get_selection_callbacks();
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

void Editor::set_status(const std::string& message, UIRenderer::StatusBarType type) {
    switch (type) {
        case UIRenderer::StatusBarType::NORMAL:
            status_bar_type = UIRenderer::StatusBarType::NORMAL;
            break;
        case UIRenderer::StatusBarType::SUCCESS:
            status_bar_type = UIRenderer::StatusBarType::SUCCESS;
            break;
        case UIRenderer::StatusBarType::ERROR:
            status_bar_type = UIRenderer::StatusBarType::ERROR;
            break;
        case UIRenderer::StatusBarType::WARNING:
            status_bar_type = UIRenderer::StatusBarType::WARNING;
            break;
        default: // not needed but just in case
            status_bar_type = UIRenderer::StatusBarType::NORMAL;
    }
    status_message = message;
    status_shown = true;
}

void Editor::delete_selection_if_active() {
    if (selection_manager.has_active_selection()) {
        delete_selection();
    }
}

std::tuple<std::function<void()>, std::function<void()>, std::function<void()>> Editor::get_selection_callbacks() {
    auto start_sel = [this]() { start_selection(); };
    auto update_sel = [this]() { update_selection(); };
    auto clear_sel = [this]() { clear_selection(); };
    return std::make_tuple(start_sel, update_sel, clear_sel);
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
    modified = true;
    set_status("Redo");
}

// ===== UI Rendering =====

Element Editor::render() {
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
    
    return ui_renderer.render(
        buffer,
        cursor_x, cursor_y,
        scroll_y,
        filename,
        modified,
        status_message,
        status_shown,
        status_bar_type,
        undo_redo_manager.can_undo(),
        undo_redo_manager.can_redo(),
        show_bold,
        show_italic,
        show_underline,
        show_strikethrough,
        is_char_selected_fn
    );
}

// ===== Event Handling =====

bool Editor::handle_event(Event event) {
    // Currently ignore all mouse events, will implement mouse later
    if (event.is_mouse()) {
        return true;
    }

    // Reset status bar variables
    status_shown = false;
    status_bar_type = UIRenderer::StatusBarType::NORMAL;
    
    // Don't reset confirm_quit if this is Ctrl+Q (char 17)
    bool is_ctrl_q = !event.is_character() && event.input().size() == 1 && 
                     (unsigned char)event.input()[0] == 17;
    if (!is_ctrl_q) {
        confirm_quit = false; // Reset quit confirmation on any other key press
    }
    
    // Check global Ctrl+C flag from signal handler
    if (ctrl_c_pressed) {
        ctrl_c_pressed = 0;
        copy_to_system_clipboard();
        return true;
    }
    
    // Handle Ctrl key combinations (Ctrl+C, Ctrl+V, Ctrl+S, etc.)
    if (!event.is_character() && event.input().size() == 1) {
        if (handle_ctrl_keys(event.input()[0])) {
            return true;
        }
    }
    
    // Handle navigation sequences (arrows with modifiers, word navigation)
    if (handle_navigation_sequences(event.input())) {
        return true;
    }
    
    // Handle standard keys (arrows, backspace, delete, enter, tab)
    if (handle_standard_keys(event)) {
        return true;
    }
    
    // Handle regular text input
    if (handle_text_input(event)) {
        return true;
    }
    
    return false;
}

bool Editor::handle_ctrl_keys(unsigned char ch) {
    switch (ch) {
        case 1:  select_all(); return true;  // Ctrl+A
        case 3:  copy_to_system_clipboard(); return true;  // Ctrl+C
        case 22: paste_from_system_clipboard(); return true;  // Ctrl+V
        case 24: cut_to_system_clipboard(); return true;  // Ctrl+X
        case 26: undo(); return true;  // Ctrl+Z
        case 25: redo(); return true;  // Ctrl+Y
        case 19: save_file(); return true;  // Ctrl+S
        case 2:  toggle_bold(); return true;  // Ctrl+B
        case 9:  toggle_italic(); return true;  // Ctrl+I
        case 21: toggle_underline(); return true;  // Ctrl+U
        case 20: toggle_strikethrough(); return true;  // Ctrl+T
        
        case 17: // Ctrl+Q
            if (modified && !confirm_quit) {
                set_status("Unsaved changes! Press Ctrl+Q again to quit.", UIRenderer::StatusBarType::WARNING);
                confirm_quit = true;
                return true;
            }
            screen->Exit();
            return true;
            
        case 15: // Ctrl+O - insert line above
            delete_selection_if_active();
            save_state();
            typing_state_saved = false;
            last_action = EditorAction::INSERT_LINE;
            buffer.insert(buffer.begin() + cursor_y, "");
            cursor_x = 0;
            modified = true;
            return true;
            
        case 11: // Ctrl+K - insert line below
            delete_selection_if_active();
            save_state();
            typing_state_saved = false;
            last_action = EditorAction::INSERT_LINE;
            buffer.insert(buffer.begin() + cursor_y + 1, "");
            cursor_y++;
            cursor_x = 0;
            modified = true;
            return true;
            
        default:
            return false;
    }
}

bool Editor::handle_navigation_sequences(const std::string& input) {

    // debug mode: CTRL + key
    if (debug_mode && input.length() == 1 && (uint8_t) input[0] <= 26) {
        std::string hex_str;
        for (unsigned char c : input) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%02x ", c);
            hex_str += buf;
        }
        set_status("CTRL Key: " + hex_str);
    } else if (debug_mode && input.length() > 1 && input[0] == '\x1b') { // Debug mode: Show escape sequences for Alt+Key and others
        std::string hex_str;
        for (unsigned char c : input) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%02x ", c);
            hex_str += buf;
        }
        set_status("Alt Key: " + hex_str);
    }
    
    // Shift + Arrow keys (selection)
    if (input == "\x1b[1;2D") { move_cursor_left(true); return true; }   // Shift+Left
    if (input == "\x1b[1;2C") { move_cursor_right(true); return true; }  // Shift+Right
    if (input == "\x1b[1;2A") { move_cursor_up(true); return true; }     // Shift+Up
    if (input == "\x1b[1;2B") { move_cursor_down(true); return true; }   // Shift+Down
    
    // Ctrl+Shift+Home/End (selection) - Works in Alacritty
    if (input == "\x1b[1;6H") { move_cursor_home(true); return true; }   // Ctrl+Shift+Home
    if (input == "\x1b[1;6F") { move_cursor_end(true); return true; }    // Ctrl+Shift+End
    if (input == "\x1b[1;5H") { move_cursor_home(true); return true; }   // Ctrl+Home with selection
    if (input == "\x1b[1;5F") { move_cursor_end(true); return true; }    // Ctrl+End with selection
    
    // Alt+Shift+Home/End (selection) - Alternative for GNOME Terminal/Kitty
    if (input == "\x1b[1;4H") { move_cursor_home(true); return true; }   // Alt+Shift+Home
    if (input == "\x1b[1;4F") { move_cursor_end(true); return true; }    // Alt+Shift+End
    if (input == "\x1b[1;3H") { move_cursor_home(true); return true; }   // Alt+Home
    if (input == "\x1b[1;3F") { move_cursor_end(true); return true; }    // Alt+End
    
    // NOTE: GNOME Terminal may intercept these keys for its own shortcuts.
    // To use in GNOME Terminal, you may need to disable terminal keybindings:
    // Preferences → Shortcuts → disable conflicting shortcuts
    
    // Shift + Home/End (selection) - May not work if terminal intercepts for scrollback
    if (input == "\x1b[1;2H") { move_cursor_home(true); return true; }   // Shift+Home (xterm)
    if (input == "\x1b[1;2F") { move_cursor_end(true); return true; }    // Shift+End (xterm)
    if (input == "\x1b[1;2~") { move_cursor_home(true); return true; }   // Shift+Home (vt100)
    if (input == "\x1b[4;2~") { move_cursor_end(true); return true; }    // Shift+End (vt100)
    
    // Ctrl+Shift+Arrow (word selection) - Alacritty
    if (input == "\x1b[1;6D") { move_word_left(true); return true; }     // Ctrl+Shift+Left
    if (input == "\x1b[1;6C") { move_word_right(true); return true; }    // Ctrl+Shift+Right
    
    // Alt+Shift+Arrow (word selection) - Universal fallback, specifically for GNOME Terminal/Kitty
    if (input == "\x1b[1;4D") { move_word_left(true); return true; }     // Alt+Shift+Left
    if (input == "\x1b[1;4C") { move_word_right(true); return true; }    // Alt+Shift+Right
    
    // Ctrl+Arrow (word navigation without selection)
    if (input == "\x1b[1;5D") { move_word_left(false); return true; }    // Ctrl+Left
    if (input == "\x1b[1;5C") { move_word_right(false); return true; }   // Ctrl+Right
    
    // ===== System Clipboard Shortcuts =====
    
    // Ctrl+Shift+V - paste from system clipboard (CSI u sequence - modern terminals)
    if (input == "\x1b[86;5u" || input == "\x1b[86;6u") {
        paste_from_system_clipboard();
        return true;
    }
    
    // Ctrl+Shift+C - copy to system clipboard (CSI u sequence - modern terminals)
    if (input == "\x1b[67;5u" || input == "\x1b[67;6u") {
        copy_to_system_clipboard();
        return true;
    }
    
    // Alt+Shift+V - paste from system clipboard (CSI u sequence)
    if (input == "\x1b[86;4u") {
        paste_from_system_clipboard();
        return true;
    }
    
    // Alt+Shift+C - copy to system clipboard (CSI u sequence)
    if (input == "\x1b[67;4u") {
        copy_to_system_clipboard();
        return true;
    }
    
    // Shift+Insert - paste from system clipboard (traditional, widely supported)
    if (input == "\x1b[2;2~") {
        paste_from_system_clipboard();
        return true;
    }
    
    // Ctrl+Insert - copy to system clipboard (traditional, widely supported)
    if (input == "\x1b[2;5~") {
        copy_to_system_clipboard();
        return true;
    }
    
    // Shift+Tab - unindent
    if (input == "\x1b[Z") {
        clear_selection();
        if (!buffer[cursor_y].empty() && buffer[cursor_y][0] == '\t') {
            save_state();
            typing_state_saved = false;
            last_action = EditorAction::UNTAB;
            buffer[cursor_y].erase(0, 1);
            if (cursor_x > 0) cursor_x--;
            modified = true;
        }
        return true;
    }
    
    return false;
}

bool Editor::handle_standard_keys(Event event) {
    // Arrow keys
    if (event == Event::ArrowLeft)  { move_cursor_left(false); return true; }
    if (event == Event::ArrowRight) { move_cursor_right(false); return true; }
    if (event == Event::ArrowUp)    { move_cursor_up(false); return true; }
    if (event == Event::ArrowDown)  { move_cursor_down(false); return true; }
    
    // Home/End keys
    if (event == Event::Home) { move_cursor_home(false); return true; }
    if (event == Event::End)  { move_cursor_end(false); return true; }
    
    // Editing keys
    if (event == Event::Backspace) { delete_char(); return true; }
    if (event == Event::Delete)    { delete_forward(); return true; }
    if (event == Event::Return)    { insert_newline(); return true; }
    
    // Tab key
    if (event == Event::Tab) {
        delete_selection_if_active();
        save_state();
        typing_state_saved = false;
        last_action = EditorAction::TAB;
        buffer[cursor_y].insert(cursor_x, "\t");
        cursor_x++;
        modified = true;
        return true;
    }
    
    return false;
}

bool Editor::handle_text_input(Event event) {
    if (!event.is_character()) return false;
    
    std::string input_str = event.input();
    
    // Skip empty input
    if (input_str.empty()) return false;
    
    // Save state before typing if we haven't already for this typing session
    if (!typing_state_saved || last_action != EditorAction::TYPING) {
        save_state();
        typing_state_saved = true;
        last_action = EditorAction::TYPING;
    }
    
    // Insert the entire UTF-8 string (could be multi-byte character)
    insert_string(input_str);
    
    return true;
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
