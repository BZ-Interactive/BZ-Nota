#include "clipboard_manager.hpp"
#include <cstdio>
#include <array>
#include <cstdlib>
#include <unistd.h>

namespace {
    constexpr size_t READ_BUFFER_SIZE = 256;
}

ClipboardManager::ClipboardManager() {}

// ===== System Clipboard Operations =====

std::string ClipboardManager::detect_clipboard_tool() const {
    // Static local variable: initialized once, persists for program lifetime (thread-safe C++11+)
    static const std::string tool = []() -> std::string {
        auto check_tool = [](const char* tool_name) -> bool {
            std::string cmd = "which " + std::string(tool_name) + " > /dev/null 2>&1";
            return system(cmd.c_str()) == 0;
        };
        
    #ifdef __APPLE__
        if (check_tool("pbcopy")) return "pbcopy";
    #else
        // Priority: Wayland > X11 (xclip > xsel)
        if (getenv("WAYLAND_DISPLAY") && check_tool("wl-copy")) return "wl-copy";
        
        if (getenv("DISPLAY")) {
            if (check_tool("xclip")) return "xclip";
            if (check_tool("xsel")) return "xsel";
        }
    #endif
        return ""; // No tool found
    }();
    return tool;
}

bool ClipboardManager::run_clipboard_command(const std::string& cmd, const std::string& input, 
                                             std::string& output, std::string& error) {
    if (!input.empty()) {
        // Write to clipboard
        FILE* pipe = popen((cmd + " 2>&1").c_str(), "w");
        if (!pipe) {
            error = "Failed to open pipe";
            return false;
        }
        
        size_t written = fwrite(input.c_str(), 1, input.size(), pipe);
        fflush(pipe);
        
        int status = pclose(pipe);
        
        if (written != input.size()) {
            error = "Failed to write all data";
            return false;
        }
        if (status != 0) {
            error = "Command exited with status " + std::to_string(status);
            return false;
        }
        return true;
    }
    
    // Read from clipboard
    FILE* pipe = popen((cmd + " 2>/dev/null").c_str(), "r");
    if (!pipe) {
        error = "Failed to open pipe";
        return false;
    }
    
    std::array<char, READ_BUFFER_SIZE> buffer;
    output.clear();
    while (fgets(buffer.data(), buffer.size(), pipe)) {
        output += buffer.data();
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        error = "Command exited with status " + std::to_string(status);
        return false;
    }
    return true;
}

bool ClipboardManager::copy_to_system(const std::string& text) {
    if (text.empty()) return false;
    
    const std::string tool = detect_clipboard_tool();
    if (tool.empty()) return false;
    
    // Map tool names to their copy commands
    std::string cmd;
    if (tool == "pbcopy") cmd = "pbcopy";
    else if (tool == "wl-copy") cmd = "wl-copy";
    else if (tool == "xclip") cmd = "xclip -selection clipboard";
    else if (tool == "xsel") cmd = "xsel --clipboard --input";
    else return false;
    
    std::string output, error;
    return run_clipboard_command(cmd, text, output, error);
}

int ClipboardManager::paste_from_system(
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y
) {
    const std::string tool = detect_clipboard_tool();
    if (tool.empty()) return -1;
    
    // Map tool names to their paste commands
    std::string cmd;
    if (tool == "pbcopy") cmd = "pbpaste";
    else if (tool == "wl-copy") cmd = "wl-paste --no-newline";
    else if (tool == "xclip") cmd = "xclip -selection clipboard -o";
    else if (tool == "xsel") cmd = "xsel --clipboard --output";
    else return -1;
    
    std::string clipboard_text, error;
    if (!run_clipboard_command(cmd, "", clipboard_text, error)) return -1;
    if (clipboard_text.empty()) return 0;
    
    // Remove single trailing newline (some tools add it)
    if (!clipboard_text.empty() && clipboard_text.back() == '\n' && 
        clipboard_text.find('\n') == clipboard_text.length() - 1) {
        clipboard_text.pop_back();
    }
    
    return insert_multiline_text(clipboard_text, buffer, cursor_x, cursor_y);
}

int ClipboardManager::insert_multiline_text(
    const std::string& text,
    std::vector<std::string>& buffer,
    int& cursor_x,
    int& cursor_y
) {
    size_t pos = 0;
    bool is_first_line = true;
    int total_chars = 0;
    
    while (true) {
        size_t newline_pos = text.find('\n', pos);
        bool has_more_lines = (newline_pos != std::string::npos);
        
        std::string line_text = has_more_lines 
            ? text.substr(pos, newline_pos - pos)
            : text.substr(pos);
        
        if (is_first_line) {
            buffer[cursor_y].insert(cursor_x, line_text);
            cursor_x += line_text.length();
            is_first_line = false;
        } else {
            std::string remainder = buffer[cursor_y].substr(cursor_x);
            buffer[cursor_y].resize(cursor_x);
            cursor_y++;
            buffer.insert(buffer.begin() + cursor_y, line_text + remainder);
            cursor_x = line_text.length();
        }
        
        total_chars += line_text.length();
        if (has_more_lines) {
            total_chars++; // Count the newline
            pos = newline_pos + 1;
        } else {
            break;
        }
    }
    
    return total_chars;
}