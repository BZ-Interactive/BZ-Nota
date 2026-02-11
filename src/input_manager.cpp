#include "input_manager.hpp"
#include "editor.hpp"
#include <cstdio>

using namespace ftxui;

InputManager::InputManager() {}

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
    
    // Don't reset confirm_quit if this is Ctrl+Q (char 17)
    bool is_ctrl_q = !event.is_character() && event.input().size() == 1 && 
                     (unsigned char)event.input()[0] == 17;
    if (!is_ctrl_q) {
        confirm_quit = false; // Reset quit confirmation on any other key press
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
        case 1:  editor.select_all(); return true;  // Ctrl+A
        case 3:  editor.copy_to_system_clipboard(); return true;  // Ctrl+C
        case 22: editor.paste_from_system_clipboard(); return true;  // Ctrl+V
        case 24: editor.cut_to_system_clipboard(); return true;  // Ctrl+X
        case 26: editor.undo(); return true;  // Ctrl+Z
        case 25: editor.redo(); return true;  // Ctrl+Y
        case 19: editor.save_file(); return true;  // Ctrl+S
        case 2:  editor.toggle_bold(); return true;  // Ctrl+B
        case 9:  editor.toggle_italic(); return true;  // Ctrl+I
        case 21: editor.toggle_underline(); return true;  // Ctrl+U
        case 20: editor.toggle_strikethrough(); return true;  // Ctrl+T
        
        case 17: // Ctrl+Q
            if (modified && !confirm_quit) {
                editor.set_status("Unsaved changes! Press Ctrl+Q again to quit.", StatusBarType::WARNING);
                confirm_quit = true;
                return true;
            }
            editor.exit();
            return true;
            
        case 15: // Ctrl+O - insert line above
            editor.delete_selection_if_active();
            editor.save_state();
            typing_state_saved = false;
            last_action = EditorAction::INSERT_LINE;
            buffer.insert(buffer.begin() + cursor_y, "");
            cursor_x = 0;
            modified = true;
            return true;
            
        case 11: // Ctrl+K - insert line below
            editor.delete_selection_if_active();
            editor.save_state();
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

bool InputManager::handle_navigation_sequences(
    const std::string& input,
    Editor& editor,
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y,
    bool& modified,
    bool debug_mode
) {
    // debug mode: CTRL + key
    if (debug_mode && input.length() == 1 && (uint8_t) input[0] <= 26) {
        std::string hex_str;
        for (unsigned char c : input) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%02x ", c);
            hex_str += buf;
        }
        editor.set_status("CTRL Key: " + hex_str);
    } else if (debug_mode && input.length() > 1 && input[0] == '\x1b') { // Debug mode: Show escape sequences for Alt+Key and others
        std::string hex_str;
        for (unsigned char c : input) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%02x ", c);
            hex_str += buf;
        }
        editor.set_status("Alt Key: " + hex_str);
    }
    
    // Shift + Arrow keys (selection)
    if (input == "\x1b[1;2D") { editor.move_cursor_left(true); return true; }   // Shift+Left
    if (input == "\x1b[1;2C") { editor.move_cursor_right(true); return true; }  // Shift+Right
    if (input == "\x1b[1;2A") { editor.move_cursor_up(true); return true; }     // Shift+Up
    if (input == "\x1b[1;2B") { editor.move_cursor_down(true); return true; }   // Shift+Down
    
    // Ctrl+Shift+Home/End (selection) - Works in Alacritty
    if (input == "\x1b[1;6H") { editor.move_cursor_home(true); return true; }   // Ctrl+Shift+Home
    if (input == "\x1b[1;6F") { editor.move_cursor_end(true); return true; }    // Ctrl+Shift+End
    if (input == "\x1b[1;5H") { editor.move_cursor_home(true); return true; }   // Ctrl+Home with selection
    if (input == "\x1b[1;5F") { editor.move_cursor_end(true); return true; }    // Ctrl+End with selection
    
    // Alt+Shift+Home/End (selection) - Alternative for GNOME Terminal/Kitty
    if (input == "\x1b[1;4H") { editor.move_cursor_home(true); return true; }   // Alt+Shift+Home
    if (input == "\x1b[1;4F") { editor.move_cursor_end(true); return true; }    // Alt+Shift+End
    if (input == "\x1b[1;3H") { editor.move_cursor_home(true); return true; }   // Alt+Home
    if (input == "\x1b[1;3F") { editor.move_cursor_end(true); return true; }    // Alt+End
    
    // NOTE: GNOME Terminal may intercept these keys for its own shortcuts.
    // To use in GNOME Terminal, you may need to disable terminal keybindings:
    // Preferences → Shortcuts → disable conflicting shortcuts
    
    // Shift + Home/End (selection) - May not work if terminal intercepts for scrollback
    if (input == "\x1b[1;2H") { editor.move_cursor_home(true); return true; }   // Shift+Home (xterm)
    if (input == "\x1b[1;2F") { editor.move_cursor_end(true); return true; }    // Shift+End (xterm)
    if (input == "\x1b[1;2~") { editor.move_cursor_home(true); return true; }   // Shift+Home (vt100)
    if (input == "\x1b[4;2~") { editor.move_cursor_end(true); return true; }    // Shift+End (vt100)
    
    // Ctrl+Shift+Arrow (word selection) - Alacritty
    if (input == "\x1b[1;6D") { editor.move_word_left(true); return true; }     // Ctrl+Shift+Left
    if (input == "\x1b[1;6C") { editor.move_word_right(true); return true; }    // Ctrl+Shift+Right
    
    // Alt+Shift+Arrow (word selection) - Universal fallback, specifically for GNOME Terminal/Kitty
    if (input == "\x1b[1;4D") { editor.move_word_left(true); return true; }     // Alt+Shift+Left
    if (input == "\x1b[1;4C") { editor.move_word_right(true); return true; }    // Alt+Shift+Right
    
    // Ctrl+Arrow (word navigation without selection)
    if (input == "\x1b[1;5D") { editor.move_word_left(false); return true; }    // Ctrl+Left
    if (input == "\x1b[1;5C") { editor.move_word_right(false); return true; }   // Ctrl+Right
    
    // ===== System Clipboard Shortcuts =====
    
    // Ctrl+Shift+V - paste from system clipboard (CSI u sequence - modern terminals)
    if (input == "\x1b[86;5u" || input == "\x1b[86;6u") {
        editor.paste_from_system_clipboard();
        return true;
    }
    
    // Ctrl+Shift+C - copy to system clipboard (CSI u sequence - modern terminals)
    if (input == "\x1b[67;5u" || input == "\x1b[67;6u") {
        editor.copy_to_system_clipboard();
        return true;
    }
    
    // Alt+Shift+V - paste from system clipboard (CSI u sequence)
    if (input == "\x1b[86;4u") {
        editor.paste_from_system_clipboard();
        return true;
    }
    
    // Alt+Shift+C - copy to system clipboard (CSI u sequence)
    if (input == "\x1b[67;4u") {
        editor.copy_to_system_clipboard();
        return true;
    }
    
    // Shift+Insert - paste from system clipboard (traditional, widely supported)
    if (input == "\x1b[2;2~") {
        editor.paste_from_system_clipboard();
        return true;
    }
    
    // Ctrl+Insert - copy to system clipboard (traditional, widely supported)
    if (input == "\x1b[2;5~") {
        editor.copy_to_system_clipboard();
        return true;
    }
    
    // Shift+Tab - unindent
    if (input == "\x1b[Z") {
        editor.clear_selection();
        if (!buffer[cursor_y].empty() && buffer[cursor_y][0] == '\t') {
            editor.save_state();
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
        typing_state_saved = false;
        last_action = EditorAction::TAB;
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
    if (!typing_state_saved || last_action != EditorAction::TYPING) {
        editor.save_state();
        typing_state_saved = true;
        last_action = EditorAction::TYPING;
    }
    
    // Insert the entire UTF-8 string (could be multi-byte character)
    editor.insert_string(input_str);
    
    return true;
}
