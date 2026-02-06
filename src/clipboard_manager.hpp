#pragma once
#include <string>
#include <vector>

/// @brief Manages system clipboard operations (cross-platform: X11, Wayland, macOS)
class ClipboardManager {
public:
    ClipboardManager();
    
    /// @brief Copy text to system clipboard
    /// @param text Text to copy
    /// @return true if successful, false otherwise
    bool copy_to_system(const std::string& text);
    
    /// @brief Paste text from system clipboard
    /// @param buffer Text buffer (modified by reference)
    /// @param cursor_x Cursor X position (modified by reference) 
    /// @param cursor_y Cursor Y position (modified by reference)
    /// @return Number of characters pasted, or -1 on error
    int paste_from_system(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );

private:
    // Cache for detected clipboard tool
    mutable std::string cached_tool;
    mutable bool tool_detected = false;
    
    // Helper methods
    std::string detect_clipboard_tool() const;
    bool run_clipboard_command(const std::string& cmd, const std::string& input, 
                               std::string& output, std::string& error);
    int insert_multiline_text(const std::string& text, std::vector<std::string>& buffer,
                              int& cursor_x, int& cursor_y);
};
