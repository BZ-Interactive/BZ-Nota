#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <tuple>

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
#include "format_manager.hpp"
#include "file_manager.hpp"

/// @brief Main text editor class - handles UI, input, and editing operations
class Editor {
public:
    Editor(const std::string& filename, bool debug_mode = false);
    ~Editor();
    
    /// @brief Start the editor main loop
    void run();
    
private:
    // Core data (std::vector = List<string>, std::string = string)
    std::vector<std::string> buffer; // Text buffer - each line is one string
    std::string filename; // Includes File path
    bool modified = false; // Has unsaved changes?
    bool status_shown = false; // Show status in UI?
    StatusBarType status_bar_type = StatusBarType::NORMAL; // Status bar type (normal, error, warning)
    std::string status_message = "";
    
    // Cursor position
    int cursor_x = 0;
    int cursor_y = 0;
    
    // Viewport
    int scroll_y = 0;
    
    // Typing state for undo
    enum class EditorAction {
        NONE,
        TYPING,
        DELETE,
        DELETE_FORWARD,
        NEWLINE,
        PASTE_SYSTEM,
        UNDO,
        REDO,
        INSERT_LINE,
        TAB,
        UNTAB
    };
    
    bool typing_state_saved = false;
    EditorAction last_action = EditorAction::NONE;
    
    // Screen reference for exiting
    ftxui::ScreenInteractive* screen = nullptr;
    
    // Quit confirmation state
    bool confirm_quit = false;
    
    // Debug mode - show key sequences in status bar
    bool debug_mode = false;
    
    // Manager instances (RAII - automatically constructed/destructed, no 'new' needed)
    // Note to self: These are actual objects, not references
    UIRenderer ui_renderer;                 // Handles all rendering
    SelectionManager selection_manager;     // Text selection state
    ClipboardManager clipboard_manager;     // Copy/paste operations
    EditingManager editing_manager;         // Insert/delete text
    CursorManager cursor_manager;           // Cursor movement
    UndoRedoManager undo_redo_manager;      // History management
    FormatManager format_manager;           // Formatting state (bold, italic, etc.)
    FileManager file_manager;               // File I/O operations
    
    // ===== File Operations =====
    void load_file();
    void save_file();
    
    // ===== Selection Operations =====
    void start_selection();
    void update_selection();
    void clear_selection();
    void delete_selection();
    void select_all();
    std::string get_selected_text();
    bool is_char_selected(int x, int y);
    
    // ===== Clipboard Operations =====
    void copy_to_system_clipboard();
    void paste_from_system_clipboard();
    void cut_to_system_clipboard();
    
    // ===== Editing Operations =====
    void insert_char(char c);
    void insert_string(const std::string& str);
    void insert_newline();
    void delete_char(); // represents backspace
    void delete_forward(); // represents delete key
    
    // ===== Formatting Operations =====
    void toggle_bold();
    void toggle_italic();
    void toggle_underline();
    void toggle_strikethrough();
    
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
    
    // Internal helpers
    void set_status(const std::string& message, StatusBarType type = StatusBarType::NORMAL);
    void delete_selection_if_active();
    std::tuple<std::function<void()>, std::function<void()>, std::function<void()>> get_selection_callbacks();
    
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
