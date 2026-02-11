#pragma once
#include <string>
#include <vector>
#include <csignal>
#include "ftxui/component/event.hpp"
#include "shared_types.hpp"

// Forward declaration to avoid circular dependency
class Editor;

/// @brief Control character constants for Ctrl+Key combinations
namespace CtrlKey {
    constexpr unsigned char A = 1;
    constexpr unsigned char B = 2;
    constexpr unsigned char C = 3;
    constexpr unsigned char I = 9;
    constexpr unsigned char K = 11;
    constexpr unsigned char O = 15;
    constexpr unsigned char Q = 17;
    constexpr unsigned char S = 19;
    constexpr unsigned char T = 20;
    constexpr unsigned char U = 21;
    constexpr unsigned char V = 22;
    constexpr unsigned char X = 24;
    constexpr unsigned char Y = 25;
    constexpr unsigned char Z = 26;
}

/// @brief Manages keyboard/mouse input events and dispatches to editor actions
/// Similar to file_manager/undo_redo_manager pattern - takes editor state as parameters
class InputManager {
public:
    InputManager();
    
    /// @brief Main event handler - dispatches to appropriate sub-handlers
    /// @param event FTXUI event to handle
    /// @param editor Reference to editor instance for calling action methods
    /// @param buffer Text buffer (for direct manipulation in some cases)
    /// @param cursor_x Cursor X position (modifiable)
    /// @param cursor_y Cursor Y position (modifiable)  
    /// @param modified Modified flag (modifiable)
    /// @param status_shown Status bar visibility flag (modifiable)
    /// @param status_bar_type Status bar type (modifiable)
    /// @param confirm_quit Quit confirmation flag (modifiable)
    /// @param debug_mode Debug mode flag (readonly)
    /// @param ctrl_c_pressed Signal flag for Ctrl+C (modifiable)
    /// @return true if event was handled, false otherwise
    bool handle_event(
        ftxui::Event event,
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
    );
    
    // Typing state for undo grouping (public so editor can access if needed)
    bool typing_state_saved = false;
    EditorAction last_action = EditorAction::NONE;
    
private:
    /// @brief Handle Ctrl+key combinations (Ctrl+C, Ctrl+V, Ctrl+S, etc.)
    bool handle_ctrl_keys(
        unsigned char ch,
        Editor& editor,
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y,
        bool& modified,
        bool& confirm_quit
    );
    
    /// @brief Handle escape/navigation sequences (arrows with modifiers, word navigation)
    bool handle_navigation_sequences(
        const std::string& input,
        Editor& editor,
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y,
        bool& modified,
        bool debug_mode
    );
    
    /// @brief Helper: Show debug info for key sequences
    void show_debug_info(const std::string& input, Editor& editor);
    
    /// @brief Helper: Insert a new line and update cursor
    void insert_line(Editor& editor, std::vector<std::string>& buffer, int& cursor_x, int& cursor_y, bool& modified, bool above);
    
    /// @brief Handle standard keys (arrows, backspace, delete, enter, tab)
    bool handle_standard_keys(
        ftxui::Event event,
        Editor& editor,
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y,
        bool& modified
    );
    
    /// @brief Handle regular text input (UTF-8 characters)
    bool handle_text_input(
        ftxui::Event event,
        Editor& editor
    );
};
