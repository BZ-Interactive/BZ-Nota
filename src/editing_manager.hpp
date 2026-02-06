#pragma once
#include <string>
#include <vector>

/// @brief Manages text editing operations (insert, delete, newline)
/// Like TextBox text manipulation methods in C#
class EditingManager {
public:
    EditingManager();
    
    // Insert character at cursor position
    // char c = single character (like C# char)
    // & parameters = modifies originals (like 'ref' in C#)
    void insert_char(
        std::vector<std::string>& buffer,  // Modifies buffer
        int& cursor_x,                      // Modifies cursor position
        int cursor_y,                       // Readonly int (passed by value)
        char c                               // Character to insert
    );
    
    // Insert UTF-8 string at cursor position
    void insert_string(
        std::vector<std::string>& buffer,  // Modifies buffer
        int& cursor_x,                      // Modifies cursor position
        int cursor_y,                       // Readonly int (passed by value)
        const std::string& str              // String to insert
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
