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
add_executable(bz-nota
    src/main.cpp
    src/editor.cpp
    src/editor.hpp
)

# Link ftxui libraries
target_link_libraries(bz-nota PRIVATE ftxui-core ftxui-screen)

# Include directories
target_include_directories(bz-nota PRIVATE src)
target_include_directories(bz-nota PRIVATE vendor/ftxui/include)

# Compiler options for better warnings and debugging
if(MSVC)
    target_compile_options(bz-nota PRIVATE /W4)
else()
    target_compile_options(bz-nota PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})