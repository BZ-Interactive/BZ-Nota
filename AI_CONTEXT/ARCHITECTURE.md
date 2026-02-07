# BZ-Nota Code Structure Documentation

## Overview
BZ-Nota is a modern terminal-based text editor built with C++ and FTXUI. This document explains the code organization to help you understand and modify the codebase.

## Project Structure

```
BZ-Nota/
├── src/
│   ├── main.cpp           # Entry point
│   ├── editor.hpp/cpp     # Main editor coordinator
│   ├── text_buffer.hpp/cpp # Text storage and file I/O
│   └── selection.hpp/cpp  # Selection state management
├── vendor/
│   └── ftxui/            # Terminal UI library (submodule)
└── CMakeLists.txt        # Build configuration
```

## Core Components

### 1. TextBuffer Class (`text_buffer.hpp/cpp`)
**Purpose**: Manages the text content - loading, saving, and line manipulation.

**Key Responsibilities**:
- Load text from files
- Save text to files
- Insert/delete characters and lines
- Provide access to text content

**Why separate?**: Separating file I/O and text storage makes the code easier to test and modify. You can change how text is stored without touching the UI code.

**Example usage**:
```cpp
TextBuffer buffer;
buffer.load_from_file("example.txt");
size_t line_count = buffer.line_count();
std::string first_line = buffer.get_line(0);
```

### 2. Selection Class (`selection.hpp/cpp`)
**Purpose**: Tracks which text is currently selected by the user.

**Key Responsibilities**:
- Track selection start and end positions
- Check if a character is within the selection
- Provide normalized selection bounds

**Why separate?**: Selection logic is complex (handling multi-line selections, reverse selections, etc.). Keeping it separate makes it easier to understand and fix bugs.

**Example usage**:
```cpp
Selection selection;
selection.start(0, 0);           // Start selecting at column 0, line 0
selection.update(10, 2);         // Select to column 10, line 2
bool selected = selection.is_char_selected(5, 1); // Check if position is selected
```

### 3. Editor Class (`editor.hpp/cpp`)
**Purpose**: Coordinates all components and handles user interaction.

**Key Responsibilities**:
- Handle keyboard input
- Render the UI
- Coordinate between TextBuffer, Selection, and clipboard
- Manage cursor position and viewport

**Why keep it?**: The Editor class is the "conductor" - it knows how all the pieces work together but delegates the detailed work to specialized classes.

## Key Concepts for C++ Beginners

### Header Files (.hpp) vs Implementation Files (.cpp)
- **Header files**: Declare what exists (class definitions, function signatures)
- **Implementation files**: Define how things work (actual function code)
- Headers are "included" where needed, implementations are compiled separately

### Class Design Principles Used

#### 1. **Single Responsibility Principle**
Each class has ONE main job:
- `TextBuffer`: Store and manage text
- `Selection`: Track selected text
- `Editor`: Coordinate everything

#### 2. **Encapsulation**
Data is kept `private` and accessed through `public` methods. This prevents accidental misuse and makes the code safer.

```cpp
class TextBuffer {
private:
    std::vector<std::string> lines;  // Can't accidentally mess this up from outside
public:
    const std::string& get_line(size_t index) const;  // Safe controlled access
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
text_buffer.cpp (100 lines)
  └── All text storage logic

selection.cpp (100 lines)
  └── All selection logic

editor.cpp (700 lines)
  └── Coordination and UI
```

### Benefits:
1. **Easier to find code**: Need to fix selection? Look in `selection.cpp`
2. **Easier to test**: Can test `TextBuffer` without the UI
3. **Easier to understand**: Each file has a clear purpose
4. **Easier to modify**: Changes to file format only affect `TextBuffer`
5. **Better for teams**: Multiple people can work on different components

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
   - Finding is about TextBuffer content → add to `text_buffer.hpp`

2. **Add declaration to header**:
```cpp
// In text_buffer.hpp
bool find_text(const std::string& search_term, size_t& line, size_t& col);
```

3. **Implement in .cpp file**:
```cpp
// In text_buffer.cpp
bool TextBuffer::find_text(const std::string& search_term, size_t& line, size_t& col) {
    for (size_t i = 0; i < lines.size(); ++i) {
        size_t pos = lines[i].find(search_term);
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
    size_t found_line, found_col;
    if (buffer.find_text("search term", found_line, found_col)) {
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
