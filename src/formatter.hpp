#pragma once
#include <string>
#include <vector>

/// @brief Represents a single formatting region with its markers and positions
struct Formatter {
    enum class Type {
        BOLD,
        ITALIC,
        UNDERLINE,
        STRIKETHROUGH
    };
    
    Type type;
    int start_index;      // Position of opening marker in line
    int end_index;        // Position AFTER closing marker in line
    int content_start;    // Position where actual content starts (after opening marker)
    int content_end;      // Position where actual content ends (before closing marker)
    std::string start_symbol;
    std::string end_symbol;
    
    Formatter(Type t, int start, int end, int c_start, int c_end, 
              const std::string& start_sym, const std::string& end_sym)
        : type(t), start_index(start), end_index(end), 
          content_start(c_start), content_end(c_end),
          start_symbol(start_sym), end_symbol(end_sym) {}
    
    /// @brief Check if a position is inside this formatter's content
    bool contains_position(int pos) const {
        return pos >= content_start && pos < content_end;
    }
    
    /// @brief Check if a position is within the markers (including markers)
    bool overlaps_position(int pos) const {
        return pos >= start_index && pos < end_index;
    }
    
    /// @brief Check if a range overlaps with this formatter
    bool overlaps_range(int range_start, int range_end) const {
        return !(range_end <= start_index || range_start >= end_index);
    }
};

/// @brief Parse formatting markers from a line of text
/// @param line The line to parse
/// @return Vector of Formatter objects representing all formatting regions
std::vector<Formatter> parse_formatters(const std::string& line);

/// @brief Adjust selection bounds to include complete formatting regions
/// @param line The line containing the selection
/// @param start Selection start position (will be modified)
/// @param end Selection end position (will be modified)
void adjust_selection_bounds(const std::string& line, int& start, int& end);
