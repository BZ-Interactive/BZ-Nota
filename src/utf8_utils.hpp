#pragma once
#include <string>

namespace UTF8Utils {
    // Get the number of bytes in a UTF-8 character starting at the given position
    int get_char_length(const std::string& str, size_t pos);
    
    // Get the number of UTF-8 characters (codepoints) in a string
    size_t char_count(const std::string& str);
    
    // Get byte position from character position
    size_t char_to_byte_pos(const std::string& str, size_t char_pos);
    
    // Get character position from byte position
    size_t byte_to_char_pos(const std::string& str, size_t byte_pos);
    
    // Check if a byte is the start of a UTF-8 character
    bool is_char_start(unsigned char byte);
    
    // Move to the next character boundary
    size_t next_char_boundary(const std::string& str, size_t pos);
    
    // Move to the previous character boundary
    size_t prev_char_boundary(const std::string& str, size_t pos);
}
