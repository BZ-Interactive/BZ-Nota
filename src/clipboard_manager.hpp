#pragma once
#include <string>
#include <vector>
#include <functional>

/// @brief Manages clipboard operations (copy, cut, paste)
/// Internal Clipboard (not system clipboard)
class ClipboardManager {
public:
    ClipboardManager();
    
    // Copy text to internal clipboard (const& = readonly reference)
    void copy(const std::string& text);
    
    // Cut text and return character count
    int cut(const std::string& text);
    
    /// @brief Paste clipboard content into buffer
    /// @param buffer Text buffer (modified by reference)
    /// @param cursor_x Cursor X position (modified by reference)
    /// @param cursor_y Cursor Y position (modified by reference)
    /// @return Number of characters pasted
    int paste(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );
    
    // Inline getters (like C# properties)
    bool is_empty() const { return clipboard.empty(); }
    const std::string& get_content() const { return clipboard; }

private:
    std::string clipboard;  // Internal clipboard (not system clipboard)
};
