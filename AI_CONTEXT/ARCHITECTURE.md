# BZ-Nota Code Structure Documentation

## Overview
BZ-Nota is a modern terminal-based text editor built with C++ and FTXUI. This document explains the code organization to help you understand and modify the codebase.

## Project Structure

```
BZ-Nota/
├── src/
│   ├── main.cpp                # Entry point
│   ├── editor.hpp/cpp          # Main editor coordinator
│   ├── selection.hpp/cpp       # Selection state
│   ├── clipboard_manager.hpp/cpp   # System clipboard operations
│   ├── selection_manager.hpp/cpp   # Selection logic and multi-line selection
│   ├── editing_manager.hpp/cpp     # Insert/delete text logic
│   ├── cursor_manager.hpp/cpp     # Cursor movement logic
│   ├── undo_redo_manager.hpp/cpp  # Undo/redo history
│   ├── format_manager.hpp/cpp     # Formatting state (bold, italic, etc.)
│   ├── ui_renderer.hpp/cpp        # UI rendering logic
├── vendor/
│   └── ftxui/                 # Terminal UI library (submodule)
└── CMakeLists.txt             # Build configuration
```

## Core Components

### 1. Editor Class (`editor.hpp/cpp`)
**Purpose**: Coordinates all components and handles user interaction.

**Key Responsibilities**:
- Handle keyboard input
- Render the UI
- Coordinate between buffer, selection, clipboard, and all manager classes
- Manage cursor position and viewport

**Why keep it?**: The Editor class is the "conductor" - it knows how all the pieces work together but delegates the detailed work to specialized classes.

**Buffer Note**: The text buffer is implemented as a `std::vector<std::string>` (one string per line) within the Editor class, with editing operations delegated to EditingManager.

### 2. Manager Classes
The editor is highly modular, with each aspect of editing delegated to a dedicated manager class:

- **UIRenderer**: Handles all UI rendering logic
- **SelectionManager**: Manages selection state, multi-line and reverse selections
- **ClipboardManager**: Handles system clipboard operations (copy, paste, cut) across platforms
- **EditingManager**: Handles text insertion, deletion, and line operations
- **CursorManager**: Handles cursor movement and navigation
- **UndoRedoManager**: Manages undo/redo history
- **FormatManager**: Tracks formatting state (bold, italic, underline, etc.)

This modular approach makes the codebase easier to maintain, test, and extend. Each manager encapsulates a single responsibility, following the Single Responsibility Principle.

### 3. Selection Class (`selection.hpp/cpp`)
**Purpose**: Tracks which text is currently selected by the user (used internally by SelectionManager).

**Key Responsibilities**:
- Track selection start and end positions
- Check if a character is within the selection
- Provide normalized selection bounds

**Why separate?**: Selection logic is complex (handling multi-line selections, reverse selections, etc.). Keeping it separate makes it easier to understand and fix bugs.

## Key Concepts for C++ Beginners

### Header Files (.hpp) vs Implementation Files (.cpp)
- **Header files**: Declare what exists (class definitions, function signatures)
- **Implementation files**: Define how things work (actual function code)
- Headers are "included" where needed, implementations are compiled separately

### Class Design Principles Used

#### 1. **Single Responsibility Principle**
Each class has ONE main job:
- `EditingManager`: Insert/delete text operations
- `Selection`: Track selected text
- `Editor`: Coordinate everything and store the buffer

#### 2. **Encapsulation**
Data is kept `private` and accessed through `public` methods. This prevents accidental misuse and makes the code safer.

```cpp
class Editor {
private:
    std::vector<std::string> buffer;  // Text buffer stored internally
public:
    const std::vector<std::string>& get_buffer() const;  // Safe controlled access
};
```

#### 3. **Clear Interfaces**
Each class has clear, well-named methods that explain what they do:
- `buffer.load_from_file()` - obvious what it does
- `selection.is_char_selected()` - obvious what it checks
- `editor.move_cursor_left()` - obvious what happens

## Code Organization Benefits

### Before Refactoring
```
editor.cpp (900+ lines)
├── File I/O mixed with cursor logic
├── Selection logic scattered throughout
├── Rendering mixed with editing
└── Hard to find specific functionality
```

### After Refactoring
```
src/
    selection.cpp (100 lines)           └── All selection logic
    clipboard_manager.cpp               └── System clipboard logic
    selection_manager.cpp               └── Selection operations
    editing_manager.cpp                 └── Editing operations
    cursor_manager.cpp                  └── Cursor movement
    undo_redo_manager.cpp               └── Undo/redo logic
    format_manager.cpp                  └── Formatting logic
    ui_renderer.cpp                     └── UI rendering
    editor.cpp (700 lines)              └── Coordination, buffer storage, and file I/O
```

### Benefits:
1. **Easier to find code**: Need to fix selection? Look in `selection_manager.cpp` or `selection.cpp`
2. **Easier to test**: Can test each manager or utility class independently
3. **Easier to understand**: Each file has a clear, focused purpose
4. **Easier to modify**: Changes to clipboard, formatting, or selection only affect the relevant manager
5. **Better for teams**: Multiple people can work on different components without conflicts

## Common Patterns

### RAII (Resource Acquisition Is Initialization)
Files are automatically closed when objects go out of scope:
```cpp
{
    std::ofstream file("data.txt");
    file << "content";
}  // file automatically closed here
```

### Const Correctness
Methods that don't modify data are marked `const`:
```cpp
const std::string& get_line(size_t index) const;
//                                        ^^^^^ - won't modify the object
```

### References vs Pointers
- References (`&`): Must always be valid, can't be null
- Pointers (`*`): Can be null, used when optional

## Building the Project

```bash
cd build
cmake ..
make
./bznota filename.txt
```

## Adding New Features

### Example: Adding a Find Feature

1. **Decide which component owns it**: 
   - Finding is about searching text → add to `editor.hpp` or create a new manager

2. **Add declaration to header**:
```cpp
// In editor.hpp
bool find_text(const std::string& search_term, int& line, int& col);
```

3. **Implement in .cpp file**:
```cpp
// In editor.cpp
bool Editor::find_text(const std::string& search_term, int& line, int& col) {
    for (size_t i = 0; i < buffer.size(); ++i) {
        size_t pos = buffer[i].find(search_term);
        if (pos != std::string::npos) {
            line = i;
            col = pos;
            return true;
        }
    }
    return false;
}
```

4. **Use in Editor**:
```cpp
// In editor.cpp handle_event()
if (event == Event::CtrlF) {
    int found_line, found_col;
    if (find_text("search term", found_line, found_col)) {
        cursor_x = found_col;
        cursor_y = found_line;
    }
}
```

## Debugging Tips

1. **Compiler errors**: Read from the first error, not all at once
2. **Missing semicolon**: Usually the line ABOVE the error
3. **Undefined reference**: Forgot to add .cpp file to CMakeLists.txt
4. **Segfault**: Usually accessing invalid memory (null pointer, out of bounds array)

## Further Reading

- FTXUI Documentation: https://github.com/ArthurSonzogni/FTXUI
- C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/
- Clean Code principles for better software design
