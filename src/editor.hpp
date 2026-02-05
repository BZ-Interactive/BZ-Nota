#pragma once
#include <string>
#include <vector>
#include <memory>

// ftxui includes - Terminal UI library
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/dom/elements.hpp"

// Local includes - manager classes
#include "ui_renderer.hpp"
#include "selection_manager.hpp"
#include "clipboard_manager.hpp"
#include "editing_manager.hpp"
#include "cursor_manager.hpp"
#include "undo_redo_manager.hpp"

/// @brief Main text editor class - handles UI, input, and editing operations
class Editor {
public:
    Editor(const std::string& filename);
    ~Editor();
    
    /// @brief Start the editor main loop
    void run();
    
private:
    // Core data (std::vector = List<string>, std::string = string)
    std::vector<std::string> buffer; // Text buffer - each line is one string
    std::string filename; // Includes File path
    bool modified = false; // Has unsaved changes?
    bool save_status_shown = false; // Show status in UI?
    std::string status_message = "";
    
    // Cursor position
    int cursor_x = 0;
    int cursor_y = 0;
    
    // Viewport
    int scroll_y = 0;
    
    // Typing state for undo
    bool typing_state_saved = false;
    std::string last_action = "";
    
    // Screen reference for exiting
    ftxui::ScreenInteractive* screen = nullptr;
    
    // Quit confirmation state
    bool confirm_quit = false;
    
    // Manager instances (RAII - automatically constructed/destructed, no 'new' needed)
    // Note to self: These are actual objects, not references
    UIRenderer ui_renderer;                 // Handles all rendering
    SelectionManager selection_manager;     // Text selection state
    ClipboardManager clipboard_manager;     // Copy/paste operations
    EditingManager editing_manager;         // Insert/delete text
    CursorManager cursor_manager;           // Cursor movement
    UndoRedoManager undo_redo_manager;      // History management
    
    // ===== File Operations =====
    void load_file();
    void save_file();
    
    // ===== Selection Operations =====
    void start_selection();
    void update_selection();
    void clear_selection();
    void delete_selection();
    std::string get_selected_text();
    bool is_char_selected(int x, int y);
    
    // ===== Clipboard Operations =====
    void copy_selection();
    void cut_selection();
    void paste_clipboard();
    
    // ===== Editing Operations =====
    void insert_char(char c);
    void insert_newline();
    void delete_char(); // represents backspace
    void delete_forward(); // represents delete key
    
    // ===== Cursor Movement =====
    void move_cursor_left(bool select = false);
    void move_cursor_right(bool select = false);
    void move_cursor_up(bool select = false);
    void move_cursor_down(bool select = false);
    void move_word_left(bool select = false);
    void move_word_right(bool select = false);
    void move_cursor_home(bool select = false);  // Home key - start of line
    void move_cursor_end(bool select = false);   // End key - end of line
    
    // ===== Helper Functions =====
    int find_word_start(int x, int y);
    int find_word_end(int x, int y);
    void ensure_cursor_visible(int screen_height);
    
    // ===== Undo/Redo =====
    void save_state();
    void undo();
    void redo();
    
    // ===== UI Rendering =====
    ftxui::Element render();
    
    // ===== Event Handling =====
    bool handle_event(ftxui::Event event);
    bool handle_ctrl_keys(unsigned char ch);
    bool handle_navigation_sequences(const std::string& input);
    bool handle_standard_keys(ftxui::Event event);
    bool handle_text_input(ftxui::Event event);
    
    // Getters (note to self: C++ doesn't have properties like C#)
    // 'const &' returns reference without copying (not to self: like 'ref readonly' in C#)
    // Trailing 'const' means method doesn't modify object
    const std::vector<std::string>& get_buffer() const { return buffer; }
    bool is_modified() const { return modified; }
};
