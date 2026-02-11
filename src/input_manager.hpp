#pragma once
#include <string>
#include <vector>
#include <functional>
#include <csignal>
#include "ftxui/component/event.hpp"
#include "shared_types.hpp"

// Forward declaration to avoid circular dependency
class Editor;

/// @brief Enum for tracking the last editor action (used by undo grouping)
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
