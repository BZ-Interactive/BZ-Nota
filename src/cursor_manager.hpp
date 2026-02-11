#pragma once
#include <string>
#include <vector>
#include <functional>  // For std::function (like Func<> or Action<> delegates in C#)

/// @brief Manages cursor movement and positioning
/// Similar to text caret in WPF TextBox but manual positioning
class CursorManager {
public:
    CursorManager();
    
    void move_left(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_right(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_up(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_down(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_word_left(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int cursor_y,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_word_right(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int cursor_y,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    // Home/End keys - move to start/end of line
    void move_home(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int cursor_y,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_end(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int cursor_y,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void ensure_cursor_visible(int cursor_y, int& scroll_y, int screen_height);
    
    // Helper functions for word boundary detection
    int find_word_start(const std::string& line, int x);
    int find_word_end(const std::string& line, int x);
    
    /// @brief Check if cursor is currently inside formatting markers
    /// @param line The current line text
    /// @param cursor_x The cursor X position
    /// @return True if cursor is between opening and closing formatting markers
    bool is_cursor_inside_formatting_markers(const std::string& line, int cursor_x);
    
    /// @brief Get the type of formatting marker at cursor position
    /// @param line The current line text
    /// @param cursor_x The cursor X position
    /// @param is_bold Output: true if inside bold markers
    /// @param is_italic Output: true if inside italic markers
    /// @param is_underline Output: true if inside underline markers
    /// @param is_strikethrough Output: true if inside strikethrough markers
    void get_formatting_at_cursor(const std::string& line, int cursor_x, 
                                  bool& is_bold, bool& is_italic, 
                                  bool& is_underline, bool& is_strikethrough);

private:
    /// @brief Skip markdown formatting markers when cursor position is inside them
    /// @param line The current line text
    /// @param cursor_x The cursor X position (will be modified to skip markers)
    /// @param direction -1 for left, +1 for right
    void skip_formatting_markers(const std::string& line, int& cursor_x, int direction);
};
