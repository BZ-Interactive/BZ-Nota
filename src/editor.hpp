#pragma once
#include <string>
#include <vector>
#include <memory>

// ftxui includes
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/dom/elements.hpp"

/// @brief Main text editor class - handles UI, input, and editing operations
class Editor {
public:
    Editor(const std::string& filename);
    ~Editor();
    
    /// @brief Start the editor main loop
    void run();
    
private:
    // Core data
    std::vector<std::string> buffer;
    std::string filename;
    bool modified = false;
    bool save_status_shown = false;
    std::string status_message = "";
    
    // Cursor position
    int cursor_x = 0;
    int cursor_y = 0;
    
    // Selection state
    bool has_selection = false;
    int selection_start_x = 0;
    int selection_start_y = 0;
    int selection_end_x = 0;
    int selection_end_y = 0;
    
    // Clipboard
    std::string clipboard;
    
    // Undo/Redo history
    struct EditorState {
        std::vector<std::string> buffer;
        int cursor_x;
        int cursor_y;
    };
    std::vector<EditorState> undo_history;
    std::vector<EditorState> redo_history;
    int max_history = 100;
    bool typing_state_saved = false;
    std::string last_action = "";
    
    // Viewport
    int scroll_y = 0;
    
    // Screen reference for exiting
    ftxui::ScreenInteractive* screen = nullptr;
    
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
    void delete_char();        // Backspace
    void delete_forward();     // Delete key
    
    // ===== Cursor Movement =====
    void move_cursor_left(bool select = false);
    void move_cursor_right(bool select = false);
    void move_cursor_up(bool select = false);
    void move_cursor_down(bool select = false);
    void move_word_left(bool select = false);
    void move_word_right(bool select =false);
    
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
    ftxui::Element render_save_button();
    ftxui::Element render_close_button();
    
    // ===== Event Handling =====
    bool handle_event(ftxui::Event event);
    bool handle_ctrl_keys(unsigned char ch);
    bool handle_navigation_sequences(const std::string& input);
    bool handle_standard_keys(ftxui::Event event);
    bool handle_text_input(ftxui::Event event);
    
    // Legacy compatibility for git repo version
    const std::vector<std::string>& get_buffer() const { return buffer; }
    bool is_modified() const { return modified; }
};
