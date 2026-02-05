#pragma once
#include <string>
#include <vector>

/// @brief Manages text selection operations
/// Similar to TextSelection class in WPF, but more manual
class SelectionManager {
public:
    SelectionManager();
    
    void start_selection(int cursor_x, int cursor_y);
    void update_selection(int cursor_x, int cursor_y);
    void clear_selection();
    
    // Inline getter (like C# property get) - const means readonly method
    bool has_active_selection() const { return has_selection; }
    bool is_char_selected(int x, int y) const;
    
    // '&' means pass by reference (modifies original, like 'ref' in C#)
    void delete_selection(
        std::vector<std::string>& buffer,  // Modifies buffer
        int& cursor_x,                      // Modifies cursor position
        int& cursor_y
    );
    
    // 'const &' means readonly reference (like 'in' parameter in C#)
    std::string get_selected_text(const std::vector<std::string>& buffer) const;
    
    // Get selection bounds - all parameters passed by reference to modify them
    void get_bounds(int& start_x, int& start_y, int& end_x, int& end_y) const;

private:
    // Private fields (like C# private fields)
    bool has_selection = false;      // In-class initializer (C++11)
    int selection_start_x = 0;
    int selection_start_y = 0;
    int selection_end_x = 0;
    int selection_end_y = 0;
};
