#include "ui_button.hpp"
#include "ftxui/dom/elements.hpp"

const ftxui::Element UIButton::spacing = ftxui::text(" ");

UIButton::UIButton(const std::string& label, ftxui::Color on_color, ftxui::Color off_color, bool active, std::function<void()> on_click)
    : label_(label),
    on_color_(on_color),
    off_color_(off_color),
    active_(active),
    dirty_(true),  // Mark as dirty initially to force first render 
    on_click_(on_click),
    previous_state_{label, active} {
}

void UIButton::set_active(bool active) {
    if (active_ != active) {
        active_ = active;
        mark_dirty();
    }
}

void UIButton::set_label(const std::string& label) {
    if (label_ != label) {
        label_ = label;
        mark_dirty();
    }
}

void UIButton::set_on_click(std::function<void()> callback) {
    on_click_ = callback;
    mark_dirty();  // Callback change might affect rendering (e.g., enabled/disabled state)
}

void UIButton::mark_dirty() {
    dirty_ = true;
}

const ftxui::Element& UIButton::render() {
    if (dirty_ || !cached_element_) {
        cached_element_ = create_element();
        clear_dirty(); // Do this AFTER creating, in case create_element throws
    }
    return cached_element_;
}

ftxui::Element UIButton::force_render() {
    cached_element_ = create_element();
    return cached_element_;
}

bool UIButton::is_dirty() const {
    return dirty_;
}

void UIButton::clear_dirty() {
    dirty_ = false;
    previous_state_.label = label_;
    previous_state_.active = active_;
}

ftxui::Element UIButton::create_element() const {
    ftxui::Element button_text = ftxui::text(label_);
    
    // Apply styling based on active state
    if (active_) {
        button_text = button_text | ftxui::color(ftxui::Color::Palette256(16)) | ftxui::bold;
    } else {
        button_text = button_text | ftxui::color(ftxui::Color::White);
    }
    
    // Add padding and border
    ftxui::Element button = ftxui::hbox({
        spacing,
        button_text,
        spacing
    })  | ftxui::bgcolor(active_ ? on_color_ : off_color_)
        | ftxui::bold;
    
    // Note: Click handler removed for now to focus on dirty flag functionality
    // FTXUI event handling would require additional setup
    
    return button;
}
