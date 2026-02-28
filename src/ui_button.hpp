#pragma once
#ifndef UI_BUTTON_HPP
#define UI_BUTTON_HPP

#include <string>
#include <optional>
#include "ftxui/dom/elements.hpp"
#include "shared_types.hpp"

// ---------------------------------------------------------------------------
// UIButtonBase
// ---------------------------------------------------------------------------
/// @brief Abstract base for all header-bar buttons.
///
///        Subclasses store the state that drives their appearance and call
///        mark_dirty() whenever that state changes.  get_element() returns a
///        cached ftxui::Element, rebuilding it only when the dirty flag is set.
class UIButtonBase {
public:
    virtual ~UIButtonBase() = default;

    /// @brief Return the (possibly cached) rendered element.
    const ftxui::Element& get_element();

protected:
    /// @brief Subclasses must implement this to produce the ftxui element.
    virtual ftxui::Element rebuild() = 0;

    /// @brief Call this whenever state that affects appearance changes.
    void mark_dirty() { _dirty = true; }

private:
    bool _dirty = true;
    ftxui::Element _cached = ftxui::emptyElement();
};

// ---------------------------------------------------------------------------
// Concrete button classes
// ---------------------------------------------------------------------------

class SaveButton : public UIButtonBase {
public:
    void set_modified(bool modified);
protected:
    ftxui::Element rebuild() override;
private:
    bool _modified = false;
};

class BoldButton : public UIButtonBase {
public:
    void set_active(bool active);
protected:
    ftxui::Element rebuild() override;
private:
    bool _active = false;
};

class ItalicButton : public UIButtonBase {
public:
    void set_active(bool active);
protected:
    ftxui::Element rebuild() override;
private:
    bool _active = false;
};

class UnderlineButton : public UIButtonBase {
public:
    void set_active(bool active);
protected:
    ftxui::Element rebuild() override;
private:
    bool _active = false;
};

class StrikethroughButton : public UIButtonBase {
public:
    void set_active(bool active);
protected:
    ftxui::Element rebuild() override;
private:
    bool _active = false;
};

class BulletButton : public UIButtonBase {
    // Stateless – built once and cached forever.
protected:
    ftxui::Element rebuild() override;
};

class UndoButton : public UIButtonBase {
public:
    void set_available(bool available);
protected:
    ftxui::Element rebuild() override;
private:
    bool _available = false;
};

class RedoButton : public UIButtonBase {
public:
    void set_available(bool available);
protected:
    ftxui::Element rebuild() override;
private:
    bool _available = false;
};

class EditorModeButton : public UIButtonBase {
public:
    void set_mode(EditorMode mode);
protected:
    ftxui::Element rebuild() override;
private:
    EditorMode _mode = EditorMode::BASIC;
};

class CloseButton : public UIButtonBase {
    // Stateless – built once and cached forever.
protected:
    ftxui::Element rebuild() override;
};

#endif // UI_BUTTON_HPP
