#pragma once
#include <string>
#include <vector>

/// @brief Manages text formatting state (bold, italic, underline, strikethrough, bullets)
class FormatManager {
public:
    FormatManager();
    
    // ===== Formatting Toggle Operations =====
    
    /// @brief Toggle bold formatting
    void toggle_bold();
    
    /// @brief Toggle italic formatting
    void toggle_italic();
    
    /// @brief Toggle underline formatting
    void toggle_underline();
    
    /// @brief Toggle strikethrough formatting
    void toggle_strikethrough();
    
    // ===== State Getters =====
    
    /// @brief Check if bold is active
    bool is_bold() const { return bold_active; }
    
    /// @brief Check if italic is active
    bool is_italic() const { return italic_active; }
    
    /// @brief Check if underline is active
    bool is_underline() const { return underline_active; }
    
    /// @brief Check if strikethrough is active
    bool is_strikethrough() const { return strikethrough_active; }
    
    /// @brief Check if any formatting is active
    bool has_active_formatting() const {
        return bold_active || italic_active || underline_active || strikethrough_active;
    }
    
    /// @brief Wrap text with active formatting markers (markdown syntax)
    /// @param text The text to wrap
    /// @return The text wrapped with appropriate markdown markers
    std::string wrap_text(const std::string& text) const;
    
    // ===== Selection Formatting =====
    
    /// @brief Wrap text with bold markers
    /// @param text The text to wrap
    /// @return The text wrapped with **text**
    std::string wrap_with_bold(const std::string& text) const;
    
    /// @brief Wrap text with italic markers
    /// @param text The text to wrap
    /// @return The text wrapped with *text*
    std::string wrap_with_italic(const std::string& text) const;
    
    /// @brief Wrap text with underline markers
    /// @param text The text to wrap
    /// @return The text wrapped with <u>text</u>
    std::string wrap_with_underline(const std::string& text) const;
    
    /// @brief Wrap text with strikethrough markers
    /// @param text The text to wrap
    /// @return The text wrapped with ~~text~~
    std::string wrap_with_strikethrough(const std::string& text) const;
    
    /// @brief Move cursor before any opening formatting markers
    /// @param line The line to check
    /// @param cursor_x Cursor position (will be modified)
    /// @return Number of positions moved back
    int move_cursor_before_opening_markers(const std::string& line, int& cursor_x) const;
    
    /// @brief Extract formatting markers and plain text from selected text
    /// @param text The text potentially containing markers
    /// @param has_bold Set to true if text contains **
    /// @param has_italic Set to true if text contains *
    /// @param has_underline Set to true if text contains <u>
    /// @param has_strikethrough Set to true if text contains ~~
    /// @return Plain text without any formatting markers
    std::string extract_formatting_from_text(const std::string& text, bool& has_bold, bool& has_italic, bool& has_underline, bool& has_strikethrough) const;
    
    // ===== Status Messages =====
    
    /// @brief Get the last status message from formatting operations
    std::string get_status_message() const { return status_message; }
    
    /// @brief Clear the status message
    void clear_status() { status_message = ""; }
    
    /// @brief Split formatting markers at cursor position
    /// @param buffer The text buffer
    /// @param cursor_x Cursor X position (will be modified)
    /// @param cursor_y Cursor Y position
    /// @param format_type "bold", "italic", "underline", or "strikethrough"
    void split_formatting_at_cursor(std::vector<std::string>& buffer, int& cursor_x, int cursor_y, const std::string& format_type);
    
    // ===== Session Management =====
    
    /// @brief Start a new formatting session (inserts opening markers)
    /// @param buffer The text buffer
    /// @param cursor_x Cursor X position (will be modified)
    /// @param cursor_y Cursor Y position
    void start_formatting_session(std::vector<std::string>& buffer, int& cursor_x, int cursor_y);
    
    /// @brief End the current formatting session (inserts closing markers)
    /// @param buffer The text buffer
    /// @param cursor_x Cursor X position (will be modified)
    /// @param cursor_y Cursor Y position
    void end_formatting_session(std::vector<std::string>& buffer, int& cursor_x, int cursor_y);
    
    /// @brief Insert both opening and closing markers, keeping cursor between them
    /// @param buffer The text buffer
    /// @param cursor_x Cursor X position (will be modified)
    /// @param cursor_y Cursor Y position
    void insert_formatting_markers(std::vector<std::string>& buffer, int& cursor_x, int cursor_y);

private:
    /// @brief Get the opening markers for currently active formatting
    std::string get_opening_markers() const;
    
    /// @brief Get the closing markers for currently active formatting (in reverse order)
    std::string get_closing_markers() const;
    
    bool bold_active = false;
    bool italic_active = false;
    bool underline_active = false;
    bool strikethrough_active = false;
    bool session_active = false;  // Track if we're in a typing session
    
    std::string status_message = "";
};
