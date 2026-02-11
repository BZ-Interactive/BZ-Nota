cmake_minimum_required(VERSION 3.10)
project(BZ-Nota VERSION 0.6.6 LANGUAGES CXX)

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
    src/format_manager.cpp
    src/format_manager.hpp
    src/file_manager.cpp
    src/file_manager.hpp
    src/input_manager.cpp
    src/input_manager.hpp
    src/formatter.cpp
    src/formatter.hpp
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

# Install icon files following freedesktop.org icon theme specification
# PNG icons for various sizes
install(FILES assets/BZ-Nota_icon_16.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps RENAME bznota.png)
install(FILES assets/BZ-Nota_icon_24.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/24x24/apps RENAME bznota.png)
install(FILES assets/BZ-Nota_icon_32.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps RENAME bznota.png)
install(FILES assets/BZ-Nota_icon_48.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps RENAME bznota.png)
install(FILES assets/BZ-Nota_icon_64.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps RENAME bznota.png)
install(FILES assets/BZ-Nota_icon_128.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps RENAME bznota.png)
install(FILES assets/BZ-Nota_icon_256.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps RENAME bznota.png)
install(FILES assets/BZ-Nota_icon_512.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/512x512/apps RENAME bznota.png)
install(FILES assets/BZ-Nota_icon_1024.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/1024x1024/apps RENAME bznota.png)

# SVG icon for scalable/vector graphics
install(FILES assets/BZ-Nota_icon_scalable.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps RENAME bznota.svg)

# Update icon cache after installation (run with appropriate privileges)
install(CODE "execute_process(COMMAND gtk-update-icon-cache -f -t ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor ERROR_QUIET)")

# Note: BZ-Nota_icon.ico is kept in assets folder for Windows packaging but not installed on Linux
