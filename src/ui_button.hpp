#pragma once
#include <string>
#include <functional>
#include <memory>
#include "ftxui/dom/elements.hpp"

/// @brief A button class that implements dirty flag pattern for efficient rendering
class UIButton {
public:
    /// @brief Constructor for UIButton
    /// @param label The text to display on the button
    /// @param active Whether the button is currently active/pressed
    /// @param bg_color Background color of the button
    /// @param fg_color Foreground color of the button
    /// @param on_click Callback function to execute when button is clicked
    UIButton(const std::string& label, ftxui::Color on_color, ftxui::Color off_color, bool active, std::function<void()> on_click = nullptr);
    
    /// @brief Update the button's active state
    /// @param active New active state
    void set_active(bool active);
    
    /// @brief Update the button's label text
    /// @param label New label text
    void set_label(const std::string& label);
    
    /// @brief Set the click callback function
    /// @param callback Function to call when button is clicked
    void set_on_click(std::function<void()> callback);
    
    /// @brief Mark the button as needing redraw (set dirty flag)
    void mark_dirty();
    
    /// @brief Render the button if it's dirty, otherwise return empty element
    /// @return FTXUI Element representing the button, or empty element if not dirty
    const ftxui::Element& render();
    
    /// @brief Force render the button regardless of dirty state
    /// @return FTXUI Element representing the button
    ftxui::Element force_render();
    
    /// @brief Check if the button needs to be redrawn
    /// @return True if button is dirty, false otherwise
    bool is_dirty() const;
    
    /// @brief Clear the dirty flag after rendering
    void clear_dirty();

private:
    std::string label_;
    ftxui::Color on_color_;
    ftxui::Color off_color_;
    bool active_;
    bool dirty_;
    std::function<void()> on_click_;
    ftxui::Element cached_element_;  // Cache the rendered element
    
    /// @brief Create the FTXUI element for the button
    /// @return The rendered button element
    ftxui::Element create_element() const;
    
    /// @brief Check if the button state has changed and mark dirty if needed
    void check_state_change();
    
    /// @brief Previous state for comparison
    struct PreviousState {
        std::string label;
        bool active;
    };
    PreviousState previous_state_;

    static const ftxui::Element spacing; // Add spacing to prevent layout issues when label changes
};