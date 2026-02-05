#pragma once
#include <string>
#include <vector>
#include <functional>

/// @brief Manages clipboard operations (copy, cut, paste)
class ClipboardManager {
public:
    ClipboardManager();
    
    /// @brief Copy text to clipboard
    void copy(const std::string& text);
    
    /// @brief Cut text to clipboard and return character count
    int cut(const std::string& text);
    
    /// @brief Paste clipboard content into buffer
    /// @return Number of characters pasted
    int paste(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );
    
    /// @brief Check if clipboard is empty
    bool is_empty() const { return clipboard.empty(); }
    
    /// @brief Get clipboard content
    const std::string& get_content() const { return clipboard; }

private:
    std::string clipboard;
};
