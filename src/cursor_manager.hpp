#pragma once
#include <string>
#include <vector>
#include <functional>  // For std::function (like Func<> or Action<> delegates in C#)

/// @brief Manages cursor movement and positioning
/// Similar to text caret in WPF TextBox but manual positioning
class CursorManager {
public:
    CursorManager();
    
    // std::function<void()> = Action in C# (delegate with no return value)
    // bool select = optional parameter with default value (like C# optional params)
    void move_left(
        const std::vector<std::string>& buffer,  // const& = readonly reference
        int& cursor_x,                           // & = modifies original (like 'ref')
        int& cursor_y,
        std::function<void()> start_selection_fn,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_right(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y,
        std::function<void()> start_selection_fn,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_up(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y,
        std::function<void()> start_selection_fn,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_down(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y,
        std::function<void()> start_selection_fn,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_word_left(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int cursor_y,
        std::function<void()> start_selection_fn,
        std::function<void()> update_selection_fn,
        std::function<void()> clear_selection_fn,
        bool select
    );
    
    void move_word_right(
        const std::vector<std::string>& buffer,
        int& cursor_x,
        int cursor_y,
        std::function<void()> start_selection_fn,
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
};
