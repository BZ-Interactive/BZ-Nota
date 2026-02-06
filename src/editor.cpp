#include "editor.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdio>   // For snprintf (debug output)
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <cstring>

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
    // Create directory if needed (C-style, could be modernized with C++17 filesystem)
    // Using raw pointers
    char* filename_copy = strdup(filename.c_str()); // Allocate memory
    char* dir = dirname(filename_copy); // Get directory path
    mkdir(dir, 0755); // Create directory
    free(filename_copy); // Free memory
    
    // RAII: file automatically closed when 'ofs' goes out of scope
    std::ofstream ofs(filename);
    if (!ofs) {  // Check if file opened successfully
        status_message = "Error: Could not save file!";
        save_status_shown = true;
        return;
    }
    
    // 'const auto&' = infer type, use const reference (avoid copy)
    for (const auto& line : buffer) {
        ofs << line << '\n';
    }
    modified = false;
    status_message = "File saved successfully";
    save_status_shown = true;
    // ofs automatically closed here (destructor called)
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

std::string Editor::get_selected_text() {
    return selection_manager.get_selected_text(buffer);
}

bool Editor::is_char_selected(int x, int y) {
    return selection_manager.is_char_selected(x, y);
}

// ===== Clipboard Operations =====

void Editor::copy_to_system_clipboard() {
    std::string text = get_selected_text();
    if (text.empty()) {
        status_message = "No text selected";
        save_status_shown = true;
        return;
    }
    
    if (clipboard_manager.copy_to_system(text)) {
        status_message = "Copied " + std::to_string(text.length()) + " chars to system clipboard";
    } else {
        status_message = "Failed to copy to system clipboard (check xclip/wl-clipboard)";
    }
    save_status_shown = true;
}

void Editor::paste_from_system_clipboard() {
    save_state();
    typing_state_saved = false;
    last_action = "paste_system";
    
    if (selection_manager.has_active_selection()) {
        delete_selection();
    }
    
    int char_count = clipboard_manager.paste_from_system(buffer, cursor_x, cursor_y);
    
    if (char_count < 0) {
        status_message = "Failed to paste from system clipboard";
        save_status_shown = true;
        return;
    }
    
    if (char_count == 0) {
        status_message = "System clipboard is empty";
    } else {
        status_message = "Pasted " + std::to_string(char_count) + " characters from system clipboard";
        modified = true;
    }
    save_status_shown = true;
}

void Editor::cut_to_system_clipboard() {
    std::string text = get_selected_text();
    if (text.empty()) {
        status_message = "No text selected";
        save_status_shown = true;
        return;
    }
    
    if (clipboard_manager.copy_to_system(text)) {
        delete_selection();
        status_message = "Cut " + std::to_string(text.length()) + " chars to system clipboard";
        modified = true;
    } else {
        status_message = "Failed to cut to system clipboard (check xclip/wl-clipboard)";
    }
    save_status_shown = true;
}

// ===== Editing Operations =====

void Editor::insert_char(char c) {
    if (selection_manager.has_active_selection()) {
        delete_selection();
    }
    editing_manager.insert_char(buffer, cursor_x, cursor_y, c);
    modified = true;
}

void Editor::insert_string(const std::string& str) {
    if (selection_manager.has_active_selection()) {
        delete_selection();
    }
    editing_manager.insert_string(buffer, cursor_x, cursor_y, str);
    modified = true;
}

void Editor::insert_newline() {
    if (selection_manager.has_active_selection()) {
        delete_selection();
    }
    
    save_state();
    typing_state_saved = false;
    last_action = "newline";
    
    editing_manager.insert_newline(buffer, cursor_x, cursor_y);
    modified = true;
}

void Editor::delete_char() {
    if (selection_manager.has_active_selection()) {
        delete_selection();
        return;
    }
    
    if (last_action != "delete") {
        save_state();
        last_action = "delete";
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
    
    if (last_action != "delete_forward") {
        save_state();
        last_action = "delete_forward";
    }
    typing_state_saved = false;
    
    editing_manager.delete_forward(buffer, cursor_x, cursor_y);
    modified = true;
}

// ===== Cursor Movement =====

void Editor::move_cursor_left(bool select) {
    auto start_sel = [this]() { this->start_selection(); };
    auto update_sel = [this]() { this->update_selection(); };
    auto clear_sel = [this]() { this->clear_selection(); };
    
    if (select && !selection_manager.has_active_selection()) {
        start_sel();
    }
    
    cursor_manager.move_left(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_cursor_right(bool select) {
    auto start_sel = [this]() { this->start_selection(); };
    auto update_sel = [this]() { this->update_selection(); };
    auto clear_sel = [this]() { this->clear_selection(); };
    
    if (select && !selection_manager.has_active_selection()) {
        start_sel();
    }
    
    cursor_manager.move_right(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_cursor_up(bool select) {
    auto start_sel = [this]() { this->start_selection(); };
    auto update_sel = [this]() { this->update_selection(); };
    auto clear_sel = [this]() { this->clear_selection(); };
    
    if (select && !selection_manager.has_active_selection()) {
        start_sel();
    }
    
    cursor_manager.move_up(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_cursor_down(bool select) {
    auto start_sel = [this]() { this->start_selection(); };
    auto update_sel = [this]() { this->update_selection(); };
    auto clear_sel = [this]() { this->clear_selection(); };
    
    if (select && !selection_manager.has_active_selection()) {
        start_sel();
    }
    
    cursor_manager.move_down(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_word_left(bool select) {
    auto start_sel = [this]() { this->start_selection(); };
    auto update_sel = [this]() { this->update_selection(); };
    auto clear_sel = [this]() { this->clear_selection(); };
    
    if (select && !selection_manager.has_active_selection()) {
        start_sel();
    }
    
    cursor_manager.move_word_left(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_word_right(bool select) {
    auto start_sel = [this]() { this->start_selection(); };
    auto update_sel = [this]() { this->update_selection(); };
    auto clear_sel = [this]() { this->clear_selection(); };
    
    if (select && !selection_manager.has_active_selection()) {
        start_sel();
    }
    
    cursor_manager.move_word_right(buffer, cursor_x, cursor_y, start_sel, update_sel, clear_sel, select);
}

void Editor::move_cursor_home(bool select) {
    auto update_sel = [this]() { this->update_selection(); };
    auto clear_sel = [this]() { this->clear_selection(); };
    
    if (select && !selection_manager.has_active_selection()) {
        start_selection();
    }
    
    cursor_manager.move_home(buffer, cursor_x, cursor_y, update_sel, clear_sel, select);
}

void Editor::move_cursor_end(bool select) {
    auto update_sel = [this]() { this->update_selection(); };
    auto clear_sel = [this]() { this->clear_selection(); };
    
    if (select && !selection_manager.has_active_selection()) {
        start_selection();
    }
    
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

// ===== Undo/Redo =====

void Editor::save_state() {
    undo_redo_manager.save_state(buffer, cursor_x, cursor_y);
}

void Editor::undo() {
    if (!undo_redo_manager.can_undo()) {
        status_message = "Nothing to undo";
        save_status_shown = true;
        return;
    }
    
    typing_state_saved = false;
    last_action = "undo";
    
    undo_redo_manager.undo(buffer, cursor_x, cursor_y);
    
    modified = true;
    status_message = "Undo";
    save_status_shown = true;
}

void Editor::redo() {
    if (!undo_redo_manager.can_redo()) {
        status_message = "Nothing to redo";
        save_status_shown = true;
        return;
    }
    
    typing_state_saved = false;
    last_action = "redo";
    
    undo_redo_manager.redo(buffer, cursor_x, cursor_y);
    
    modified = true;
    status_message = "Redo";
    save_status_shown = true;
}

// ===== UI Rendering =====

Element Editor::render() {
    int screen_height = Terminal::Size().dimy;
    ensure_cursor_visible(screen_height);
    
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
        save_status_shown,
        undo_redo_manager.can_undo(),
        undo_redo_manager.can_redo(),
        is_char_selected_fn
    );
}

// ===== Event Handling =====

bool Editor::handle_event(Event event) {
    save_status_shown = false;
    
    // Don't reset confirm_quit if this is Ctrl+Q (char 17)
    bool is_ctrl_q = !event.is_character() && event.input().size() == 1 && 
                     (unsigned char)event.input()[0] == 17;
    if (!is_ctrl_q) {
        confirm_quit = false; // Reset quit confirmation on any other key press
    }
    
    // Ignore all mouse events
    if (event.is_mouse()) {
        return true;
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
        case 3:  // Ctrl+C - System clipboard
            copy_to_system_clipboard();
            return true;
            
        case 22: // Ctrl+V - System clipboard
            paste_from_system_clipboard();
            return true;
            
        case 24: // Ctrl+X - System clipboard
            cut_to_system_clipboard();
            return true;
            
        case 26: // Ctrl+Z
            undo();
            return true;
            
        case 25: // Ctrl+Y
            redo();
            return true;
            
        case 19: // Ctrl+S
            save_file();
            return true;
            
        case 17: // Ctrl+Q
            if (modified && !confirm_quit) {
                status_message = "Unsaved changes! Press Ctrl+Q again to quit.";
                save_status_shown = true;
                confirm_quit = true;
                return true;
            }
            screen->Exit();
            return true;
            
        case 15: // Ctrl+O - insert line above
            if (selection_manager.has_active_selection()) delete_selection();
            save_state();
            typing_state_saved = false;
            last_action = "insert_line";
            buffer.insert(buffer.begin() + cursor_y, "");
            cursor_x = 0;
            modified = true;
            return true;
            
        case 11: // Ctrl+K - insert line below
            if (selection_manager.has_active_selection()) delete_selection();
            save_state();
            typing_state_saved = false;
            last_action = "insert_line";
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
    // Debug mode: Show ALL escape sequences (enable with -d flag)
    if (debug_mode && input.length() > 1 && input[0] == '\x1b') {
        status_message = "Key: ";
        for (unsigned char c : input) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%02x ", c);
            status_message += buf;
        }
        save_status_shown = true;
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
    
    // Alt+Shift+Home/End (selection) - Alternative for GNOME Terminal
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
    
    // Alt+Shift+Arrow (word selection) - Universal fallback
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
    
    // Shift+Tab - unindent (remove leading tab if present)
    if (input == "\x1b[Z") {
        if (selection_manager.has_active_selection()) clear_selection();
        if (!buffer[cursor_y].empty() && buffer[cursor_y][0] == '\t') {
            save_state();
            typing_state_saved = false;
            last_action = "untab";
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
        if (selection_manager.has_active_selection()) delete_selection();
        save_state();
        typing_state_saved = false;
        last_action = "tab";
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
    if (!typing_state_saved || last_action != "typing") {
        save_state();
        typing_state_saved = true;
        last_action = "typing";
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
