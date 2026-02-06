cmake_minimum_required(VERSION 3.10)
project(BZ-Nota VERSION 0.4.2 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Generate version header from template
configure_file(src/version.hpp.in src/version.hpp @ONLY)

# Generate .desktop file from template
configure_file(bznota.desktop.in bznota.desktop @ONLY)

# Add ftxui as a subdirectory (submodule)
add_subdirectory(vendor/ftxui)

# Create the main executable
add_executable(bznota
    src/main.cpp
    src/editor.cpp
    src/editor.hpp
    src/text_buffer.cpp
    src/text_buffer.hpp
    src/selection.cpp
    src/selection.hpp
    src/ui_renderer.cpp
    src/ui_renderer.hpp
    src/selection_manager.cpp
    src/selection_manager.hpp
    src/clipboard_manager.cpp
    src/clipboard_manager.hpp
    src/editing_manager.cpp
    src/editing_manager.hpp
    src/cursor_manager.cpp
    src/cursor_manager.hpp
    src/undo_redo_manager.cpp
    src/undo_redo_manager.hpp
    src/utf8_utils.cpp
    src/utf8_utils.hpp
)

# Link ftxui libraries
target_link_libraries(bznota PRIVATE ftxui::screen ftxui::dom ftxui::component)

# Include directories
target_include_directories(bznota PRIVATE src)
target_include_directories(bznota PRIVATE ${CMAKE_BINARY_DIR}/src)  # For generated version.hpp
target_include_directories(bznota PRIVATE vendor/ftxui/include)

# Compiler options for better warnings and debugging
if(MSVC)
    target_compile_options(bznota PRIVATE /W4)
else()
    target_compile_options(bznota PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Standard installation rules
include(GNUInstallDirs)

install(TARGETS bznota RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install .desktop file for GUI integration
install(FILES ${CMAKE_BINARY_DIR}/bznota.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
