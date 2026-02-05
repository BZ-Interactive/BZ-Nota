#pragma once
#include <string>
#include <vector>
#include <functional>

/// @brief Manages cursor movement and positioning
class CursorManager {
public:
    CursorManager();
    
    void move_left(
        const std::vector<std::string>& buffer,
        int& cursor_x,
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
    
    void ensure_cursor_visible(int cursor_y, int& scroll_y, int screen_height);
    
    // Helper functions
    int find_word_start(const std::string& line, int x);
    int find_word_end(const std::string& line, int x);
};
