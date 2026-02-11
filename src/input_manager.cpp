#include "input_manager.hpp"
#include "editor.hpp"
#include <sstream>
#include <iomanip>
#include <fstream>

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
    
    // Handle rename mode input
    if (is_renaming) {
        return handle_rename_input(event, editor);
    }
    
    // Handle function keys (F1, F2, etc.)
    if (handle_fn_keys(event, editor)) {
        return true;
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

bool InputManager::handle_fn_keys(ftxui::Event event, Editor& editor) {
    // F1: Help
    if (event == Event::F1) {
        editor.set_status("Fn Help: F1-Help, F2-Rename, F5-Reload", StatusBarType::NORMAL);
        return true;
    } 
    // F2: Start rename mode
    else if (event == Event::F2) {
        // Extract just the filename (remove path)
        std::string basename = editor.filename;
        size_t last_slash = basename.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            basename = basename.substr(last_slash + 1);
        }
        
        // Initialize rename mode
        is_renaming = true;
        rename_input = basename;
        editor.set_status("Rename file to: " + rename_input + " (Enter to confirm, Esc to cancel)", StatusBarType::WARNING);
        return true;
    }
    else if (event == Event::F5) {
        // 1. Reset all 'locked' UI states
        is_renaming = false;
        rename_input = "";
        is_confirming_overwrite = false;
        pending_rename_target = "";
        
        // 2. Clear visual glitches
        // This forces the terminal to wipe and redraw the whole grid
        editor.screen_reset(); 


        return true;
    }
    
    return false;
}

bool InputManager::handle_rename_input(ftxui::Event event, Editor& editor) {
    // Handle overwrite confirmation (y/n)
    if (is_confirming_overwrite) {
        if (event.is_character()) {
            std::string input = event.input();
            if (input == "y" || input == "Y") {
                // User confirmed - proceed with rename
                editor.rename_file(pending_rename_target);
                is_renaming = false;
                is_confirming_overwrite = false;
                rename_input.clear();
                pending_rename_target.clear();
                return true;
            } else if (input == "n" || input == "N" || event == Event::Escape) {
                // User cancelled
                editor.set_status("Rename cancelled", StatusBarType::NORMAL);
                is_renaming = false;
                is_confirming_overwrite = false;
                rename_input.clear();
                pending_rename_target.clear();
                return true;
            }
        }
        return true; // Ignore other keys during confirmation
    }
    
    // Handle Enter - confirm rename
    if (event == Event::Return) {
        if (rename_input.empty()) {
            editor.set_status("Cannot rename to empty filename!", StatusBarType::ERROR);
            is_renaming = false;
            rename_input.clear();
            return true;
        }
        
        // Get the directory path from the current filename
        std::string dirpath = "";
        size_t last_slash = editor.filename.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            dirpath = editor.filename.substr(0, last_slash + 1);
        }
        
        // Build full path for new filename
        std::string new_fullpath = dirpath + rename_input;
        
        // Check if target file already exists
        std::ifstream test_file(new_fullpath);
        if (test_file.good()) {
            test_file.close();
            // File exists - ask for confirmation
            pending_rename_target = new_fullpath;
            is_confirming_overwrite = true;
            editor.set_status("File '" + rename_input + "' already exists! Overwrite? (y/n)", StatusBarType::WARNING);
            return true;
        }
        
        // File doesn't exist - proceed with rename
        editor.rename_file(new_fullpath);
        
        // Exit rename mode
        is_renaming = false;
        rename_input.clear();
        return true;
    }
    
    // Handle Escape - cancel rename
    if (event == Event::Escape) {
        editor.set_status("Rename cancelled", StatusBarType::NORMAL);
        is_renaming = false;
        is_confirming_overwrite = false;
        rename_input.clear();
        pending_rename_target.clear();
        return true;
    }
    
    // Handle Backspace
    if (event == Event::Backspace) {
        if (!rename_input.empty()) {
            rename_input.pop_back();
        }
        // Always update status, even if empty (to show it's still in rename mode)
        editor.set_status("Rename file to: " + rename_input + " (Enter to confirm, Esc to cancel)", StatusBarType::WARNING);
        return true;
    }
    
    // Handle regular character input
    if (event.is_character()) {
        std::string input_str = event.input();
        
        // Filter out invalid filename characters (basic validation)
        bool valid = true;
        for (char c : input_str) {
            if (c == '/' || c == '\\' || c == '\0') {
                valid = false;
                break;
            }
        }
        
        if (valid && !input_str.empty()) {
            rename_input += input_str;
            editor.set_status("Rename file to: " + rename_input + " (Enter to confirm, Esc to cancel)", StatusBarType::WARNING);
        }
        return true;
    }
    
    // Ignore other keys during rename mode
    return true;
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
