#pragma once
#include <string>
#include <vector>

/// @brief Manages the text buffer - loading, saving, and manipulating lines of text
class TextBuffer {
public:
    TextBuffer();
    
    /// @brief Load content from a file
    /// @param filename Path to the file to load
    /// @return true if file was loaded, false if new file
    bool load_from_file(const std::string& filename);
    
    /// @brief Save content to a file
    /// @param filename Path to the file to save
    /// @return true if save was successful
    bool save_to_file(const std::string& filename);
    
    /// @brief Get a line from the buffer
    /// @param line_index Index of the line (0-based)
    /// @return Reference to the line
    const std::string& get_line(size_t line_index) const;
    
    /// @brief Get mutable access to a line
    /// @param line_index Index of the line (0-based)
    /// @return Reference to the line
    std::string& get_line_mut(size_t line_index);
    
    /// @brief Get the total number of lines
    size_t line_count() const;
    
    /// @brief Insert a character at a position
    void insert_char(size_t line, size_t col, char c);
    
    /// @brief Delete a character at a position
    void delete_char(size_t line, size_t col);
    
    /// @brief Insert a new line
    void insert_line(size_t line_index, const std::string& content = "");
    
    /// @brief Delete a line
    void delete_line(size_t line_index);
    
    /// @brief Split a line at a position
    void split_line(size_t line, size_t col);
    
    /// @brief Join current line with next line
    void join_lines(size_t line);
    
    /// @brief Check if buffer is empty
    bool is_empty() const;
    
    /// @brief Get all lines (for iteration)
    const std::vector<std::string>& get_lines() const;

private:
    std::vector<std::string> lines;
};
