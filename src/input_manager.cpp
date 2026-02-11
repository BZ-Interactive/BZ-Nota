#include "input_manager.hpp"
#include "editor.hpp"
#include <sstream>
#include <iomanip>

using namespace ftxui;

InputManager::InputManager() {}

// ===================== Helper Functions =====================

void InputManager::show_debug_info(const std::string& input, Editor& editor) {
    std::ostringstream oss;
    
    // Format hex string
    for (unsigned char c : input) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c << " ";
    }
    
    // Determine key type
    if (input.length() == 1 && (uint8_t)input[0] <= 26) {
        editor.set_status("CTRL Key: " + oss.str());
    } else if (input.length() > 1 && input[0] == '\x1b') {
        editor.set_status("Alt Key: " + oss.str());
    }
}

void InputManager::insert_line(Editor& editor, std::vector<std::string>& buffer, 
                               int& cursor_x, int& cursor_y, bool& modified, bool above) {
    editor.delete_selection_if_active();
    editor.save_state();
    editor.typing_state_saved = false;
    editor.last_action = EditorAction::INSERT_LINE;
    
    if (above) {
        buffer.insert(buffer.begin() + cursor_y, "");
    } else {
        buffer.insert(buffer.begin() + cursor_y + 1, "");
        cursor_y++;
    }
    
    cursor_x = 0;
    modified = true;
}

// ===================== Main Event Handler =====================

bool InputManager::handle_event(
    Event event,
    Editor& editor,
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y,
    bool& modified,
    bool& status_shown,
    StatusBarType& status_bar_type,
    bool& confirm_quit,
    bool debug_mode,
    volatile sig_atomic_t& ctrl_c_pressed
) {
    // Currently ignore all mouse events, will implement mouse later
    if (event.is_mouse()) {
        return true;
    }

    // Reset status bar variables
    status_shown = false;
    status_bar_type = StatusBarType::NORMAL;
    
    // Don't reset confirm_quit if this is Ctrl+Q
    bool is_ctrl_q = !event.is_character() && event.input().size() == 1 && 
                     (unsigned char)event.input()[0] == CtrlKey::Q;
    if (!is_ctrl_q) {
        confirm_quit = false;
    }
    
    // Check global Ctrl+C flag from signal handler
    if (ctrl_c_pressed) {
        ctrl_c_pressed = 0;
        editor.copy_to_system_clipboard();
        return true;
    }
    
    // Handle Ctrl key combinations (Ctrl+C, Ctrl+V, Ctrl+S, etc.)
    if (!event.is_character() && event.input().size() == 1) {
        if (handle_ctrl_keys(event.input()[0], editor, buffer, cursor_x, cursor_y, modified, confirm_quit)) {
            return true;
        }
    }
    
    // Handle navigation sequences (arrows with modifiers, word navigation)
    if (handle_navigation_sequences(event.input(), editor, buffer, cursor_x, cursor_y, modified, debug_mode)) {
        return true;
    }
    
    // Handle standard keys (arrows, backspace, delete, enter, tab)
    if (handle_standard_keys(event, editor, buffer, cursor_x, cursor_y, modified)) {
        return true;
    }
    
    // Handle regular text input
    if (handle_text_input(event, editor)) {
        return true;
    }
    
    return false;
}

bool InputManager::handle_ctrl_keys(
    unsigned char ch,
    Editor& editor,
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y,
    bool& modified,
    bool& confirm_quit
) {
    switch (ch) {
        // Clipboard operations
        case CtrlKey::A: editor.select_all(); return true;
        case CtrlKey::C: editor.copy_to_system_clipboard(); return true;
        case CtrlKey::V: editor.paste_from_system_clipboard(); return true;
        case CtrlKey::X: editor.cut_to_system_clipboard(); return true;
        
        // Undo/Redo
        case CtrlKey::Z: editor.undo(); return true;
        case CtrlKey::Y: editor.redo(); return true;
        
        // File operations
        case CtrlKey::S: editor.save_file(); return true;
        
        // Formatting
        case CtrlKey::B: editor.toggle_bold(); return true;
        case CtrlKey::I: editor.toggle_italic(); return true;
        case CtrlKey::U: editor.toggle_underline(); return true;
        case CtrlKey::T: editor.toggle_strikethrough(); return true;
        
        // Line operations
        case CtrlKey::O: insert_line(editor, buffer, cursor_x, cursor_y, modified, true); return true;
        case CtrlKey::K: insert_line(editor, buffer, cursor_x, cursor_y, modified, false); return true;
        
        // Quit
        case CtrlKey::Q:
            if (modified && !confirm_quit) {
                editor.set_status("Unsaved changes! Press Ctrl+Q again to quit.", StatusBarType::WARNING);
                confirm_quit = true;
                return true;
            }
            editor.exit();
            return true;
            
        default:
            return false;
    }
}

bool InputManager::handle_navigation_sequences(
    const std::string& input,
    Editor& editor,
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y,
    bool& modified,
    bool debug_mode
) {
    // Debug mode: show key sequence hex codes
    if (debug_mode) {
        show_debug_info(input, editor);
    }
    
    // ===== Arrow Navigation with Modifiers =====
    
    // Shift+Arrow: Selection
    if (input == "\x1b[1;2D") { editor.move_cursor_left(true); return true; }
    if (input == "\x1b[1;2C") { editor.move_cursor_right(true); return true; }
    if (input == "\x1b[1;2A") { editor.move_cursor_up(true); return true; }
    if (input == "\x1b[1;2B") { editor.move_cursor_down(true); return true; }
    
    // Ctrl+Arrow: Word navigation
    if (input == "\x1b[1;5D") { editor.move_word_left(false); return true; }
    if (input == "\x1b[1;5C") { editor.move_word_right(false); return true; }
    
    // Ctrl+Shift+Arrow: Word selection (Alacritty)
    if (input == "\x1b[1;6D") { editor.move_word_left(true); return true; }
    if (input == "\x1b[1;6C") { editor.move_word_right(true); return true; }
    
    // Alt+Shift+Arrow: Word selection (GNOME Terminal/Kitty fallback)
    if (input == "\x1b[1;4D") { editor.move_word_left(true); return true; }
    if (input == "\x1b[1;4C") { editor.move_word_right(true); return true; }
    
    // ===== Home/End Navigation with Modifiers =====
    
    // Shift+Home/End: Selection (xterm)
    if (input == "\x1b[1;2H") { editor.move_cursor_home(true); return true; }
    if (input == "\x1b[1;2F") { editor.move_cursor_end(true); return true; }
    
    // Shift+Home/End: Selection (vt100)
    if (input == "\x1b[1;2~") { editor.move_cursor_home(true); return true; }
    if (input == "\x1b[4;2~") { editor.move_cursor_end(true); return true; }
    
    // Ctrl+Shift+Home/End: Selection (Alacritty)
    if (input == "\x1b[1;6H") { editor.move_cursor_home(true); return true; }
    if (input == "\x1b[1;6F") { editor.move_cursor_end(true); return true; }
    
    // Ctrl+Home/End: Selection
    if (input == "\x1b[1;5H") { editor.move_cursor_home(true); return true; }
    if (input == "\x1b[1;5F") { editor.move_cursor_end(true); return true; }
    
    // Alt+Shift+Home/End: Selection (GNOME Terminal/Kitty)
    if (input == "\x1b[1;4H") { editor.move_cursor_home(true); return true; }
    if (input == "\x1b[1;4F") { editor.move_cursor_end(true); return true; }
    
    // Alt+Home/End: Selection
    if (input == "\x1b[1;3H") { editor.move_cursor_home(true); return true; }
    if (input == "\x1b[1;3F") { editor.move_cursor_end(true); return true; }
    
    // ===== System Clipboard Shortcuts =====
    
    // Ctrl+Shift+C/V: Modern terminals (CSI u sequence)
    if (input == "\x1b[67;5u" || input == "\x1b[67;6u") { editor.copy_to_system_clipboard(); return true; }
    if (input == "\x1b[86;5u" || input == "\x1b[86;6u") { editor.paste_from_system_clipboard(); return true; }
    
    // Alt+Shift+C/V: Alternative for some terminals
    if (input == "\x1b[67;4u") { editor.copy_to_system_clipboard(); return true; }
    if (input == "\x1b[86;4u") { editor.paste_from_system_clipboard(); return true; }
    
    // Ctrl+Insert / Shift+Insert: Traditional shortcuts
    if (input == "\x1b[2;5~") { editor.copy_to_system_clipboard(); return true; }
    if (input == "\x1b[2;2~") { editor.paste_from_system_clipboard(); return true; }
    
    // ===== Other Special Keys =====
    
    // Shift+Tab: Unindent
    if (input == "\x1b[Z") {
        editor.clear_selection();
        if (!buffer[cursor_y].empty() && buffer[cursor_y][0] == '\t') {
            editor.save_state();
            editor.typing_state_saved = false;
            editor.last_action = EditorAction::UNTAB;
            buffer[cursor_y].erase(0, 1);
            if (cursor_x > 0) cursor_x--;
            modified = true;
        }
        return true;
    }
    
    return false;
}

bool InputManager::handle_standard_keys(
    Event event,
    Editor& editor,
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y,
    bool& modified
) {
    // Arrow keys
    if (event == Event::ArrowLeft)  { editor.move_cursor_left(false); return true; }
    if (event == Event::ArrowRight) { editor.move_cursor_right(false); return true; }
    if (event == Event::ArrowUp)    { editor.move_cursor_up(false); return true; }
    if (event == Event::ArrowDown)  { editor.move_cursor_down(false); return true; }
    
    // Home/End keys
    if (event == Event::Home) { editor.move_cursor_home(false); return true; }
    if (event == Event::End)  { editor.move_cursor_end(false); return true; }
    
    // Editing keys
    if (event == Event::Backspace) { editor.delete_char(); return true; }
    if (event == Event::Delete)    { editor.delete_forward(); return true; }
    if (event == Event::Return)    { editor.insert_newline(); return true; }
    
    // Tab key
    if (event == Event::Tab) {
        editor.delete_selection_if_active();
        editor.save_state();
        editor.typing_state_saved = false;
        editor.last_action = EditorAction::TAB;
        buffer[cursor_y].insert(cursor_x, "\t");
        cursor_x++;
        modified = true;
        return true;
    }
    
    return false;
}

bool InputManager::handle_text_input(
    Event event,
    Editor& editor
) {
    if (!event.is_character()) return false;
    
    std::string input_str = event.input();
    
    // Skip empty input
    if (input_str.empty()) return false;
    
    // Save state before typing if we haven't already for this typing session
    if (!editor.typing_state_saved || editor.last_action != EditorAction::TYPING) {
        editor.save_state();
        editor.typing_state_saved = true;
        editor.last_action = EditorAction::TYPING;
    }
    
    // Insert the entire UTF-8 string (could be multi-byte character)
    editor.insert_string(input_str);
    
    return true;
}
