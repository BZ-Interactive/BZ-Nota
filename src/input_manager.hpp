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
    bool handle_event(ftxui::Event event, Editor& editor, volatile sig_atomic_t& ctrl_c_pressed);
    
private:
    bool is_renaming = false; // State for F2 rename operation
    std::string rename_input; // Buffer for F2 rename input
    bool is_confirming_overwrite = false; // State for overwrite confirmation
    std::string pending_rename_target; // Full path of pending rename target

    /// @brief Handle Ctrl+key combinations (Ctrl+C, Ctrl+V, Ctrl+S, etc.)
    bool handle_ctrl_keys(unsigned char ch, Editor& editor);
    
    /// @brief Handle escape/navigation sequences (arrows with modifiers, word navigation)
    bool handle_navigation_sequences(const std::string& input, Editor& editor, bool debug_mode);
    
    /// @brief Handle function keys (F1, F2, etc.)
    bool handle_fn_keys(ftxui::Event event, Editor& editor);
    
    /// @brief Handle text input during rename mode (F2)
    bool handle_rename_input(ftxui::Event event, Editor& editor);

    /// @brief Helper: Show debug info for key sequences
    void show_debug_info(const std::string& input, Editor& editor);

    /// @brief Handle standard keys (arrows, backspace, delete, enter, tab)
    bool handle_standard_keys(ftxui::Event event, Editor& editor);
    
    /// @brief Handle regular text input (UTF-8 characters)
    bool handle_text_input(ftxui::Event event, Editor& editor);

};
