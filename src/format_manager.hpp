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
    
    // ===== Status Messages =====
    
    /// @brief Get the last status message from formatting operations
    std::string get_status_message() const { return status_message; }
    
    /// @brief Clear the status message
    void clear_status() { status_message = ""; }
    
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
    
    /// @brief Check if we're currently in a formatting session
    bool in_formatting_session() const { return session_active; }
    
    /// @brief Get the opening markers for current active formatting
    std::string get_opening_markers() const;
    
    /// @brief Get the closing markers for current active formatting
    std::string get_closing_markers() const;

private:
    bool bold_active = false;
    bool italic_active = false;
    bool underline_active = false;
    bool strikethrough_active = false;
    bool session_active = false;  // Track if we're in a typing session
    
    std::string status_message = "";
};
