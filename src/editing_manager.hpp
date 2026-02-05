#pragma once
#include <string>
#include <vector>

/// @brief Manages text editing operations (insert, delete, newline)
class EditingManager {
public:
    EditingManager();
    
    void insert_char(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int cursor_y,
        char c
    );
    
    void insert_newline(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );
    
    void delete_char(
        std::vector<std::string>& buffer,
        int& cursor_x,
        int& cursor_y
    );
    
    void delete_forward(
        std::vector<std::string>& buffer,
        int cursor_x,
        int cursor_y
    );
};
