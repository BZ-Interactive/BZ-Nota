#include "formatter.hpp"
#include <algorithm>

std::vector<Formatter> parse_formatters(const std::string& line) {
    std::vector<Formatter> formatters;
    
    // Find all bold regions **...**
    size_t pos = 0;
    while (pos < line.length()) {
        pos = line.find("**", pos);
        if (pos == std::string::npos) break;
        
        size_t close_pos = line.find("**", pos + 2);
        if (close_pos != std::string::npos) {
            formatters.emplace_back(
                Formatter::Type::BOLD,
                pos,                    // start_index
                close_pos + 2,         // end_index (after closing **)
                pos + 2,               // content_start
                close_pos,             // content_end
                "**",
                "**"
            );
            pos = close_pos + 2;
        } else {
            pos += 2;
        }
    }
    
    // Find all underline regions <u>...</u>
    pos = 0;
    while (pos < line.length()) {
        pos = line.find("<u>", pos);
        if (pos == std::string::npos) break;
        
        size_t close_pos = line.find("</u>", pos + 3);
        if (close_pos != std::string::npos) {
            formatters.emplace_back(
                Formatter::Type::UNDERLINE,
                pos,                    // start_index
                close_pos + 4,         // end_index (after closing </u>)
                pos + 3,               // content_start
                close_pos,             // content_end
                "<u>",
                "</u>"
            );
            pos = close_pos + 4;
        } else {
            pos += 3;
        }
    }
    
    // Find all strikethrough regions ~~...~~
    pos = 0;
    while (pos < line.length()) {
        pos = line.find("~~", pos);
        if (pos == std::string::npos) break;
        
        size_t close_pos = line.find("~~", pos + 2);
        if (close_pos != std::string::npos) {
            formatters.emplace_back(
                Formatter::Type::STRIKETHROUGH,
                pos,                    // start_index
                close_pos + 2,         // end_index (after closing ~~)
                pos + 2,               // content_start
                close_pos,             // content_end
                "~~",
                "~~"
            );
            pos = close_pos + 2;
        } else {
            pos += 2;
        }
    }
    
    // Find all italic regions *...*  (but not **)
    pos = 0;
    while (pos < line.length()) {
        pos = line.find('*', pos);
        if (pos == std::string::npos) break;
        
        // Skip if it's part of **
        if (pos + 1 < line.length() && line[pos + 1] == '*') {
            pos += 2;
            continue;
        }
        if (pos > 0 && line[pos - 1] == '*') {
            pos++;
            continue;
        }
        
        // Find closing *
        size_t close_pos = pos + 1;
        while (close_pos < line.length()) {
            close_pos = line.find('*', close_pos);
            if (close_pos == std::string::npos) break;
            
            // Make sure it's not part of **
            if (close_pos + 1 < line.length() && line[close_pos + 1] == '*') {
                close_pos += 2;
                continue;
            }
            if (close_pos > 0 && line[close_pos - 1] == '*') {
                close_pos++;
                continue;
            }
            
            // Found valid closing *
            formatters.emplace_back(
                Formatter::Type::ITALIC,
                pos,                    // start_index
                close_pos + 1,         // end_index (after closing *)
                pos + 1,               // content_start
                close_pos,             // content_end
                "*",
                "*"
            );
            pos = close_pos + 1;
            break;
        }
        
        if (close_pos == std::string::npos) {
            pos++;
        }
    }
    
    // Sort formatters by start position
    std::sort(formatters.begin(), formatters.end(), 
        [](const Formatter& a, const Formatter& b) {
            return a.start_index < b.start_index;
        });
    
    return formatters;
}

void adjust_selection_bounds(const std::string& line, int& start, int& end) {
    if (start < 0 || end < 0 || start >= (int)line.length()) return;
    
    // Parse all formatters in the line
    std::vector<Formatter> formatters = parse_formatters(line);
    
    // Find all formatters that overlap with the selection
    std::vector<const Formatter*> overlapping;
    for (const auto& fmt : formatters) {
        if (fmt.overlaps_range(start, end)) {
            overlapping.push_back(&fmt);
        }
    }
    
    // Expand selection to include all overlapping formatters completely
    for (const auto* fmt : overlapping) {
        if (fmt->start_index < start) {
            start = fmt->start_index;
        }
        if (fmt->end_index > end) {
            end = fmt->end_index;
        }
    }
}
