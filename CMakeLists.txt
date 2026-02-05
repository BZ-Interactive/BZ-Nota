cmake_minimum_required(VERSION 3.10)
project(BZ-Nota VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

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
)

# Link ftxui libraries
target_link_libraries(bznota PRIVATE ftxui::screen ftxui::dom ftxui::component)

# Include directories
target_include_directories(bznota PRIVATE src)
target_include_directories(bznota PRIVATE vendor/ftxui/include)

# Compiler options for better warnings and debugging
if(MSVC)
    target_compile_options(bznota PRIVATE /W4)
else()
    target_compile_options(bznota PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})