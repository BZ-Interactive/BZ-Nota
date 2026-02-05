#pragma once

/// @brief Manages text selection state
class Selection {
public:
    Selection();
    
    /// @brief Start a new selection from current position
    void start(int x, int y);
    
    /// @brief Update selection end position
    void update(int x, int y);
    
    /// @brief Clear the selection
    void clear();
    
    /// @brief Check if there is an active selection
    bool is_active() const;
    
    /// @brief Check if a character position is selected
    bool is_char_selected(int x, int y) const;
    
    /// @brief Get selection bounds (normalized)
    void get_bounds(int& start_x, int& start_y, int& end_x, int& end_y) const;
    
    /// @brief Get the start position
    void get_start(int& x, int& y) const;
    
    /// @brief Get the end position
    void get_end(int& x, int& y) const;

private:
    bool active = false;
    int start_x = 0;
    int start_y = 0;
    int end_x = 0;
    int end_y = 0;
};
