#include "utf8_utils.hpp"
#include <algorithm>

namespace UTF8Utils {

// Get the number of bytes in a UTF-8 character starting at the given position
int get_char_length(const std::string& str, size_t pos) {
    if (pos >= str.length()) return 0;
    
    unsigned char c = static_cast<unsigned char>(str[pos]);
    
    // Single-byte character (ASCII): 0xxxxxxx
    if ((c & 0x80) == 0) return 1;
    
    // Two-byte character: 110xxxxx 10xxxxxx
    if ((c & 0xE0) == 0xC0) return 2;
    
    // Three-byte character: 1110xxxx 10xxxxxx 10xxxxxx
    if ((c & 0xF0) == 0xE0) return 3;
    
    // Four-byte character: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if ((c & 0xF8) == 0xF0) return 4;
    
    // Invalid UTF-8 start byte, treat as single byte
    return 1;
}

// Get the number of UTF-8 characters (codepoints) in a string
size_t char_count(const std::string& str) {
    size_t count = 0;
    size_t pos = 0;
    
    while (pos < str.length()) {
        int len = get_char_length(str, pos);
        pos += len;
        count++;
    }
    
    return count;
}

// Get byte position from character position
size_t char_to_byte_pos(const std::string& str, size_t char_pos) {
    size_t byte_pos = 0;
    size_t current_char = 0;
    
    while (byte_pos < str.length() && current_char < char_pos) {
        int len = get_char_length(str, byte_pos);
        byte_pos += len;
        current_char++;
    }
    
    return byte_pos;
}

// Get character position from byte position
size_t byte_to_char_pos(const std::string& str, size_t byte_pos) {
    size_t char_pos = 0;
    size_t current_byte = 0;
    
    while (current_byte < byte_pos && current_byte < str.length()) {
        int len = get_char_length(str, current_byte);
        current_byte += len;
        char_pos++;
    }
    
    return char_pos;
}

// Check if a byte is the start of a UTF-8 character
bool is_char_start(unsigned char byte) {
    // Start byte: 0xxxxxxx or 11xxxxxx (not 10xxxxxx)
    return (byte & 0x80) == 0 || (byte & 0xC0) == 0xC0;
}

// Move to the next character boundary
size_t next_char_boundary(const std::string& str, size_t pos) {
    if (pos >= str.length()) return pos;
    
    int len = get_char_length(str, pos);
    return std::min(pos + len, str.length());
}

// Move to the previous character boundary
size_t prev_char_boundary(const std::string& str, size_t pos) {
    if (pos == 0) return 0;
    
    size_t new_pos = pos - 1;
    
    // Keep moving back until we find a character start byte
    while (new_pos > 0 && !is_char_start(static_cast<unsigned char>(str[new_pos]))) {
        new_pos--;
    }
    
    return new_pos;
}

} // namespace UTF8Utils
