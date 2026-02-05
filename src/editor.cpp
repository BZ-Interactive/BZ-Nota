#include "editor.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <cstring>

using namespace ftxui;

// Global signal handler flag
static volatile sig_atomic_t ctrl_c_pressed = 0;

static void handle_sigint(int sig) {
    (void)sig;
    ctrl_c_pressed = 1;
}

// ===== Constructor / Destructor =====

Editor::Editor(const std::string& fn) : filename(fn) {
    load_file();
}

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
        status_message = "Error: Could not save file!";
        save_status_shown = true;
        return;
    }
    
    for (const auto& line : buffer) {
        ofs << line << '\n';
    }
    modified = false;
    status_message = "File saved successfully";
    save_status_shown = true;
}

// ===== Selection Operations =====

void Editor::start_selection() {
    has_selection = true;
    selection_start_x = cursor_x;
    selection_start_y = cursor_y;
    selection_end_x = cursor_x;
    selection_end_y = cursor_y;
}

void Editor::update_selection() {
    if (has_selection) {
        selection_end_x = cursor_x;
        selection_end_y = cursor_y;
    }
}

void Editor::clear_selection() {
    has_selection = false;
}

void Editor::delete_selection() {
    if (!has_selection) return;
    
    save_state();
    int start_y = std::min(selection_start_y, selection_end_y);
    int end_y = std::max(selection_start_y, selection_end_y);
    int start_x = selection_start_x;
    int end_x = selection_end_x;
    
    if (selection_start_y > selection_end_y || 
        (selection_start_y == selection_end_y && selection_start_x > selection_end_x)) {
        std::swap(start_x, end_x);
    }
    
    if (start_y == end_y) {
        // Single line deletion
        buffer[start_y].erase(start_x, end_x - start_x);
        cursor_x = start_x;
        cursor_y = start_y;
    } else {
        // Multi-line deletion
        std::string remaining = buffer[start_y].substr(0, start_x) + 
                               buffer[end_y].substr(end_x);
        buffer[start_y] = remaining;
        buffer.erase(buffer.begin() + start_y + 1, buffer.begin() + end_y + 1);
        cursor_x = start_x;
        cursor_y = start_y;
    }
    
    clear_selection();
    modified = true;
}

std::string Editor::get_selected_text() {
    if (!has_selection) return "";
    
    int start_y = std::min(selection_start_y, selection_end_y);
    int end_y = std::max(selection_start_y, selection_end_y);
    int start_x = selection_start_x;
    int end_x = selection_end_x;
    
    if (selection_start_y > selection_end_y || 
        (selection_start_y == selection_end_y && selection_start_x > selection_end_x)) {
        std::swap(start_x, end_x);
    }
    
    if (start_y == end_y) {
        // Single line
        return buffer[start_y].substr(start_x, end_x - start_x);
    } else {
        // Multi-line
        std::string result = buffer[start_y].substr(start_x) + "\n";
        for (int y = start_y + 1; y < end_y; y++) {
            result += buffer[y] + "\n";
        }
        result += buffer[end_y].substr(0, end_x);
        return result;
    }
}

bool Editor::is_char_selected(int x, int y) {
    if (!has_selection) return false;
    
    int start_y = std::min(selection_start_y, selection_end_y);
    int end_y = std::max(selection_start_y, selection_end_y);
    int start_x = selection_start_x;
    int end_x = selection_end_x;
    
    if (selection_start_y > selection_end_y || 
        (selection_start_y == selection_end_y && selection_start_x > selection_end_x)) {
        std::swap(start_x, end_x);
    }
    
    if (y < start_y || y > end_y) return false;
    if (y == start_y && y == end_y) {
        return x >= start_x && x < end_x;
    }
    if (y == start_y) {
        return x >= start_x;
    }
    if (y == end_y) {
        return x < end_x;
    }
    return true;
}

// ===== Clipboard Operations =====

void Editor::copy_selection() {
    clipboard = get_selected_text();
    if (!clipboard.empty()) {
        int char_count = clipboard.size();
        status_message = "Copied " + std::to_string(char_count) + " characters";
        save_status_shown = true;
    }
}

void Editor::cut_selection() {
    clipboard = get_selected_text();
    if (!clipboard.empty()) {
        int char_count = clipboard.size();
        delete_selection();
        status_message = "Cut " + std::to_string(char_count) + " characters";
        save_status_shown = true;
    }
}

void Editor::paste_clipboard() {
    if (clipboard.empty()) return;
    
    save_state();
    typing_state_saved = false;
    last_action = "paste";
    
    if (has_selection) {
        delete_selection();
    }
    
    size_t pos = 0;
    size_t newline_pos;
    bool first_line = true;
    
    while ((newline_pos = clipboard.find('\n', pos)) != std::string::npos) {
        std::string line_to_insert = clipboard.substr(pos, newline_pos - pos);
        
        if (first_line) {
            buffer[cursor_y].insert(cursor_x, line_to_insert);
            cursor_x += line_to_insert.length();
            first_line = false;
        } else {
            std::string remainder = buffer[cursor_y].substr(cursor_x);
            buffer[cursor_y] = buffer[cursor_y].substr(0, cursor_x);
            cursor_y++;
            buffer.insert(buffer.begin() + cursor_y, line_to_insert + remainder);
            cursor_x = line_to_insert.length();
        }
        
        pos = newline_pos + 1;
    }
    
    // Insert remaining text (no newline at end)
    if (pos < clipboard.length()) {
        std::string remaining = clipboard.substr(pos);
        buffer[cursor_y].insert(cursor_x, remaining);
        cursor_x += remaining.length();
    }
    
    int char_count = clipboard.size();
    status_message = "Pasted " + std::to_string(char_count) + " characters";
    save_status_shown = true;
    modified = true;
}

// ===== Editing Operations =====

void Editor::insert_char(char c) {
    if (has_selection) {
        delete_selection();
    }
    buffer[cursor_y].insert(cursor_x, 1, c);
    cursor_x++;
    modified = true;
}

void Editor::insert_newline() {
    if (has_selection) {
        delete_selection();
    }
    
    save_state();
    typing_state_saved = false;
    last_action = "newline";
    
    std::string current_line = buffer[cursor_y];
    std::string before_cursor = current_line.substr(0, cursor_x);
    std::string after_cursor = current_line.substr(cursor_x);
    
    buffer[cursor_y] = before_cursor;
    buffer.insert(buffer.begin() + cursor_y + 1, after_cursor);
    
    cursor_y++;
    cursor_x = 0;
    modified = true;
}

void Editor::delete_char() {
    if (has_selection) {
        delete_selection();
        return;
    }
    
    if (last_action != "delete") {
        save_state();
        last_action = "delete";
    }
    typing_state_saved = false;
    
    if (cursor_x > 0) {
        buffer[cursor_y].erase(cursor_x - 1, 1);
        cursor_x--;
        modified = true;
    } else if (cursor_y > 0) {
        cursor_x = buffer[cursor_y - 1].length();
        buffer[cursor_y - 1] += buffer[cursor_y];
        buffer.erase(buffer.begin() + cursor_y);
        cursor_y--;
        modified = true;
    }
}

void Editor::delete_forward() {
    if (has_selection) {
        delete_selection();
        return;
    }
    
    if (last_action != "delete_forward") {
        save_state();
        last_action = "delete_forward";
    }
    typing_state_saved = false;
    
    if (cursor_x < (int)buffer[cursor_y].length()) {
        buffer[cursor_y].erase(cursor_x, 1);
        modified = true;
    } else if (cursor_y < (int)buffer.size() - 1) {
        buffer[cursor_y] += buffer[cursor_y + 1];
        buffer.erase(buffer.begin() + cursor_y + 1);
        modified = true;
    }
}

// ===== Cursor Movement =====

void Editor::move_cursor_left(bool select) {
    if (select && !has_selection) {
        start_selection();
    }
    
    if (cursor_x > 0) {
        cursor_x--;
    } else if (cursor_y > 0) {
        cursor_y--;
        cursor_x = buffer[cursor_y].length();
    }
    
    if (select) {
        update_selection();
    } else {
        clear_selection();
    }
}

void Editor::move_cursor_right(bool select) {
    if (select && !has_selection) {
        start_selection();
    }
    
    if (cursor_x < (int)buffer[cursor_y].length()) {
        cursor_x++;
    } else if (cursor_y < (int)buffer.size() - 1) {
        cursor_y++;
        cursor_x = 0;
    }
    
    if (select) {
        update_selection();
    } else {
        clear_selection();
    }
}

void Editor::move_cursor_up(bool select) {
    if (select && !has_selection) {
        start_selection();
    }
    
    if (cursor_y > 0) {
        cursor_y--;
        cursor_x = std::min(cursor_x, (int)buffer[cursor_y].length());
    }
    
    if (select) {
        update_selection();
    } else {
        clear_selection();
    }
}

void Editor::move_cursor_down(bool select) {
    if (select && !has_selection) {
        start_selection();
    }
    
    if (cursor_y < (int)buffer.size() - 1) {
        cursor_y++;
        cursor_x = std::min(cursor_x, (int)buffer[cursor_y].length());
    }
    
    if (select) {
        update_selection();
    } else {
        clear_selection();
    }
}

void Editor::move_word_left(bool select) {
    if (select && !has_selection) {
        start_selection();
    }
    
    int new_x = find_word_start(cursor_x, cursor_y);
    cursor_x = new_x;
    
    if (select) {
        update_selection();
    } else {
        clear_selection();
    }
}

void Editor::move_word_right(bool select) {
    if (select && !has_selection) {
        start_selection();
    }
    
    int new_x = find_word_end(cursor_x, cursor_y);
    cursor_x = new_x;
    
    if (select) {
        update_selection();
    } else {
        clear_selection();
    }
}

// ===== Helper Functions =====

int Editor::find_word_start(int x, int y) {
    const std::string& line = buffer[y];
    
    if (x == 0) return 0;
    
    // Skip whitespace
    while (x > 0 && !std::isalnum(line[x - 1]) && line[x - 1] != '_') {
        x--;
    }
    
    // Skip word characters
    while (x > 0 && (std::isalnum(line[x - 1]) || line[x - 1] == '_')) {
        x--;
    }
    
    return x;
}

int Editor::find_word_end(int x, int y) {
    const std::string& line = buffer[y];
    int len = line.length();
    
    if (x >= len) return len;
    
    // Skip whitespace
    while (x < len && !std::isalnum(line[x]) && line[x] != '_') {
        x++;
    }
    
    // Skip word characters
    while (x < len && (std::isalnum(line[x]) || line[x] == '_')) {
        x++;
    }
    
    return x;
}

void Editor::ensure_cursor_visible(int screen_height) {
    int visible_lines = screen_height - 3; // Account for header/status
    
    if (cursor_y < scroll_y) {
        scroll_y = cursor_y;
    } else if (cursor_y >= scroll_y + visible_lines) {
        scroll_y = cursor_y - visible_lines + 1;
    }
}

// ===== Undo/Redo =====

void Editor::save_state() {
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

void Editor::undo() {
    if (undo_history.empty()) {
        status_message = "Nothing to undo";
        save_status_shown = true;
        return;
    }
    
    typing_state_saved = false;
    last_action = "undo";
    
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
    
    modified = true;
    status_message = "Undo";
    save_status_shown = true;
}

void Editor::redo() {
    if (redo_history.empty()) {
        status_message = "Nothing to redo";
        save_status_shown = true;
        return;
    }
    
    typing_state_saved = false;
    last_action = "redo";
    
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
        is_char_selected_fn
    );
}

// ===== Event Handling =====

bool Editor::handle_event(Event event) {
    save_status_shown = false;
    confirm_quit = false; // Reset quit confirmation on any key press
    
    // Ignore all mouse events
    if (event.is_mouse()) {
        return true;
    }
    
    // Check global Ctrl+C flag from signal handler
    if (ctrl_c_pressed) {
        ctrl_c_pressed = 0;
        copy_selection();
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
        case 3:  // Ctrl+C
            copy_selection();
            return true;
            
        case 22: // Ctrl+V
            paste_clipboard();
            return true;
            
        case 24: // Ctrl+X
            cut_selection();
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
            if (has_selection) delete_selection();
            save_state();
            typing_state_saved = false;
            last_action = "insert_line";
            buffer.insert(buffer.begin() + cursor_y, "");
            cursor_x = 0;
            modified = true;
            return true;
            
        case 11: // Ctrl+K - insert line below
            if (has_selection) delete_selection();
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
    // Shift + Arrow keys (selection)
    if (input == "\x1b[1;2D") { move_cursor_left(true); return true; }   // Shift+Left
    if (input == "\x1b[1;2C") { move_cursor_right(true); return true; }  // Shift+Right
    if (input == "\x1b[1;2A") { move_cursor_up(true); return true; }     // Shift+Up
    if (input == "\x1b[1;2B") { move_cursor_down(true); return true; }   // Shift+Down
    
    // Ctrl+Shift+Arrow (word selection) - Alacritty
    if (input == "\x1b[1;6D") { move_word_left(true); return true; }     // Ctrl+Shift+Left
    if (input == "\x1b[1;6C") { move_word_right(true); return true; }    // Ctrl+Shift+Right
    
    // Alt+Shift+Arrow (word selection) - Universal fallback
    if (input == "\x1b[1;4D") { move_word_left(true); return true; }     // Alt+Shift+Left
    if (input == "\x1b[1;4C") { move_word_right(true); return true; }    // Alt+Shift+Right
    
    // Ctrl+Arrow (word navigation without selection)
    if (input == "\x1b[1;5D") { move_word_left(false); return true; }    // Ctrl+Left
    if (input == "\x1b[1;5C") { move_word_right(false); return true; }   // Ctrl+Right
    
    // Ctrl+Shift+V in Alacritty - treat as paste
    if (input == "\x1b[86;5u" || input == "\x1b[86;6u") {
        paste_clipboard();
        return true;
    }
    
    // Shift+Tab - unindent (remove leading tab if present)
    if (input == "\x1b[Z") {
        if (has_selection) clear_selection();
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
    
    // Editing keys
    if (event == Event::Backspace) { delete_char(); return true; }
    if (event == Event::Delete)    { delete_forward(); return true; }
    if (event == Event::Return)    { insert_newline(); return true; }
    
    // Tab key
    if (event == Event::Tab) {
        if (has_selection) delete_selection();
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
    
    // Save state before typing if we haven't already for this typing session
    if (!typing_state_saved || last_action != "typing") {
        save_state();
        typing_state_saved = true;
        last_action = "typing";
    }
    
    // Insert printable characters
    for (char c : input_str) {
        if (c >= 32 && c < 127) {
            insert_char(c);
        }
    }
    
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
