#include "ui_button.hpp"
#include "ftxui/dom/elements.hpp"
#include <cstdlib>  // std::getenv

using namespace ftxui;

// ---------------------------------------------------------------------------
// Shared helpers (mirrors what UIRenderer had)
// ---------------------------------------------------------------------------

// UI Color Constants - Disabled/Inactive State
static const Color BUTTON_DISABLED_BG_PRIMARY   = Color::GrayDark;
static const Color BUTTON_DISABLED_BG_SECONDARY = Color::Black;
static const Color BUTTON_DISABLED_FG           = Color::White;

// UI Color Constants - Active State
static const Color BUTTON_ACTIVE_FG = Color::Black;

// Button-Specific Colors (Active/Enabled State)
static const Color SAVE_BUTTON_ACTIVE_BG          = Color::BlueLight;
static const Color BOLD_BUTTON_ACTIVE_BG          = Color::Orange3;
static const Color ITALIC_BUTTON_ACTIVE_BG        = Color::Pink3;
static const Color UNDERLINE_BUTTON_ACTIVE_BG     = Color::Green4;
static const Color STRIKETHROUGH_BUTTON_ACTIVE_BG = Color::Red3;
static const Color BULLET_BUTTON_BG               = Color::Black;
static const Color BULLET_BUTTON_FG               = Color::White;
static const Color UNDO_BUTTON_ACTIVE_BG          = Color::DarkOrange;
static const Color REDO_BUTTON_ACTIVE_BG          = Color::GreenLight;
static const Color CLOSE_BUTTON_BG                = Color::GrayLight;
static const Color CLOSE_BUTTON_FG                = Color::RedLight;

static const Color EDITOR_MODE_BASIC_BG    = Color::White;
static const Color EDITOR_MODE_BASIC_FG    = Color::Black;
static const Color EDITOR_MODE_FANCY_BG    = Color::SeaGreen1Bis;
static const Color EDITOR_MODE_FANCY_FG    = Color::Black;
static const Color EDITOR_MODE_CODE_BG     = Color::Magenta2Bis;
static const Color EDITOR_MODE_CODE_FG     = Color::White;
static const Color EDITOR_MODE_DOCUMENT_BG = Color::NavyBlue;
static const Color EDITOR_MODE_DOCUMENT_FG = Color::White;

static const Element sp = text(" ");

/// @brief Detect emoji support once at startup (same logic as UIRenderer::supports_emojis).
static bool detect_emoji_support() {
    const char* colorterm  = std::getenv("COLORTERM");
    const char* term       = std::getenv("TERM");
    const char* wt_session = std::getenv("WT_SESSION");
    const char* wt_profile = std::getenv("WT_PROFILE_ID");
    std::string term_str   = term ? term : "";

    if (wt_session || wt_profile) return true;

    if (colorterm && (std::string(colorterm) == "truecolor" ||
                      std::string(colorterm) == "24bit")) {
        return true;
    }

    if (term_str == "alacritty" || term_str == "xterm-kitty" || term_str == "foot") {
        return true;
    }

    return false;
}

static bool emoji_support() {
    static const bool val = detect_emoji_support();
    return val;
}

// ---------------------------------------------------------------------------
// UIButtonBase
// ---------------------------------------------------------------------------

const Element& UIButtonBase::get_element() {
    if (_dirty) {
        _cached = rebuild();
        _dirty  = false;
    }
    return _cached;
}

// ---------------------------------------------------------------------------
// SaveButton
// ---------------------------------------------------------------------------

void SaveButton::set_modified(bool modified) {
    if (modified != _modified) {
        _modified = modified;
        mark_dirty();
    }
}

Element SaveButton::rebuild() {
    auto symbol = emoji_support() ? text("💾") | nothing : text("⌼") | bold;
    return hbox({sp, symbol, text(" Ctrl+S ") | nothing}) |
           bgcolor(_modified ? SAVE_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_PRIMARY) |
           color(_modified ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (_modified ? bold : nothing);
}

// ---------------------------------------------------------------------------
// BoldButton
// ---------------------------------------------------------------------------

void BoldButton::set_active(bool active) {
    if (active != _active) {
        _active = active;
        mark_dirty();
    }
}

Element BoldButton::rebuild() {
    auto symbol = emoji_support() ? text("🅱️") | nothing : text("B") | bold;
    return hbox({sp, symbol, text(" Alt+B ") | nothing}) |
           bgcolor(_active ? BOLD_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_SECONDARY) |
           color(_active ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (_active ? bold : nothing);
}

// ---------------------------------------------------------------------------
// ItalicButton
// ---------------------------------------------------------------------------

void ItalicButton::set_active(bool active) {
    if (active != _active) {
        _active = active;
        mark_dirty();
    }
}

Element ItalicButton::rebuild() {
    auto symbol = text("I") | bold | italic;
    return hbox({sp, symbol, text(" Alt+I ") | nothing}) |
           bgcolor(_active ? ITALIC_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_PRIMARY) |
           color(_active ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (_active ? bold : nothing);
}

// ---------------------------------------------------------------------------
// UnderlineButton
// ---------------------------------------------------------------------------

void UnderlineButton::set_active(bool active) {
    if (active != _active) {
        _active = active;
        mark_dirty();
    }
}

Element UnderlineButton::rebuild() {
    auto symbol = text("U") | bold | underlined;
    return hbox({sp, symbol, text(" Alt+U ") | nothing}) |
           bgcolor(_active ? UNDERLINE_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_SECONDARY) |
           color(_active ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (_active ? bold : nothing);
}

// ---------------------------------------------------------------------------
// StrikethroughButton
// ---------------------------------------------------------------------------

void StrikethroughButton::set_active(bool active) {
    if (active != _active) {
        _active = active;
        mark_dirty();
    }
}

Element StrikethroughButton::rebuild() {
    auto symbol = text(" S ") | bold | strikethrough;
    return hbox({symbol, text(" Alt+T ") | nothing}) |
           bgcolor(_active ? STRIKETHROUGH_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_PRIMARY) |
           color(_active ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (_active ? bold : nothing);
}

// ---------------------------------------------------------------------------
// BulletButton  (stateless – cached after first build)
// ---------------------------------------------------------------------------

Element BulletButton::rebuild() {
    auto symbol = text("•") | bold;
    return hbox({sp, symbol, text(" Alt+[0-9] ") | nothing}) |
           bgcolor(BULLET_BUTTON_BG) |
           color(BULLET_BUTTON_FG);
}

// ---------------------------------------------------------------------------
// UndoButton
// ---------------------------------------------------------------------------

void UndoButton::set_available(bool available) {
    if (available != _available) {
        _available = available;
        mark_dirty();
    }
}

Element UndoButton::rebuild() {
    auto symbol = text("↩️");
    return hbox({sp, symbol, text(" Ctrl+Z ") | nothing}) |
           bgcolor(_available ? UNDO_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_PRIMARY) |
           color(_available ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (_available ? bold : nothing);
}

// ---------------------------------------------------------------------------
// RedoButton
// ---------------------------------------------------------------------------

void RedoButton::set_available(bool available) {
    if (available != _available) {
        _available = available;
        mark_dirty();
    }
}

Element RedoButton::rebuild() {
    auto symbol = text("↪️");
    return hbox({sp, symbol, text(" Ctrl+Y ") | nothing}) |
           bgcolor(_available ? REDO_BUTTON_ACTIVE_BG : BUTTON_DISABLED_BG_SECONDARY) |
           color(_available ? BUTTON_ACTIVE_FG : BUTTON_DISABLED_FG) |
           (_available ? bold : nothing);
}

// ---------------------------------------------------------------------------
// EditorModeButton
// ---------------------------------------------------------------------------

void EditorModeButton::set_mode(EditorMode mode) {
    if (mode != _mode) {
        _mode = mode;
        mark_dirty();
    }
}

Element EditorModeButton::rebuild() {
    std::string mode_text;
    Color bg_color;
    Color fg_color;

    switch (_mode) {
        case EditorMode::BASIC:
            mode_text = "Mode: Basic";
            bg_color  = EDITOR_MODE_BASIC_BG;
            fg_color  = EDITOR_MODE_BASIC_FG;
            break;
        case EditorMode::FANCY:
            mode_text = "Mode: Fancy";
            bg_color  = EDITOR_MODE_FANCY_BG;
            fg_color  = EDITOR_MODE_FANCY_FG;
            break;
        case EditorMode::CODE:
            mode_text = "Mode: Code";
            bg_color  = EDITOR_MODE_CODE_BG;
            fg_color  = EDITOR_MODE_CODE_FG;
            break;
        case EditorMode::DOCUMENT:
            mode_text = "Mode: Document";
            bg_color  = EDITOR_MODE_DOCUMENT_BG;
            fg_color  = EDITOR_MODE_DOCUMENT_FG;
            break;
    }

    return hbox({sp, text(mode_text), text(" F7 ") | nothing}) |
           bgcolor(bg_color) |
           color(fg_color) |
           bold;
}

// ---------------------------------------------------------------------------
// CloseButton  (stateless – cached after first build)
// ---------------------------------------------------------------------------

Element CloseButton::rebuild() {
    auto symbol = emoji_support() ? text("❌") | nothing : text("X") | bold;
    return hbox({sp, symbol, text(" Ctrl+Q ") | nothing}) |
           bgcolor(CLOSE_BUTTON_BG) |
           color(CLOSE_BUTTON_FG) |
           bold;
}
