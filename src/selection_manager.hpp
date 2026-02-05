#pragma once
#include <string>
#include <vector>

/// @brief Manages text selection operations
class SelectionManager {
public:
    SelectionManager();
    
    void start_selection(int cursor_x, int cursor_y);
    void update_selection(int cursor_x, int cursor_y);
    void clear_selection();
    
    bool has_active_selection() const { return has_selection; }
    bool is_char_selected(int x, int y) const;
    
    void delete_selection(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );
    
    std::string get_selected_text(const std::vector<std::string>& buffer) const;
    
    // Get selection bounds
    void get_bounds(int& start_x, int& start_y, int& end_x, int& end_y) const;

private:
    bool has_selection = false;
    int selection_start_x = 0;
    int selection_start_y = 0;
    int selection_end_x = 0;
    int selection_end_y = 0;
};
