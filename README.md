<!-- Improved README.md generation -->
<br />
<div align="center">
  <table>
    <tr>
      <td valign="middle" style="padding-right:10px">
        <a href="https://github.com/BZ-Interactive/BZ-Nota">
          <img src="assets/BZ-Nota_icon_scalable.svg" alt="BZ-Nota logo" width="200" height="200">
        </a>
      </td>
      <td valign="middle" style="text-align:left; max-width:720px">
        <h3 style="margin:0">BZ-Nota</h3>
        <p style="margin-top:8px; margin-bottom:8px; text-align:left">
          A high-performance, terminal-based text editor built on C++. It combines the intuitive editing of Micro with a modern, btop-style dashboard interface for a modern TUI experience.
        </p>
        <p style="margin:0; text-align:left">
          <a href="https://github.com/BZ-Interactive/BZ-Nota"><strong>Explore the docs »</strong></a>
          &nbsp;&nbsp;|&nbsp;&nbsp;
          <a href="https://github.com/BZ-Interactive/BZ-Nota/releases">See Releases</a>
          &nbsp;·&nbsp;
          <a href="https://github.com/BZ-Interactive/BZ-Nota/issues">Request Feature</a>
          &nbsp;·&nbsp;
          <a href="https://github.com/BZ-Interactive/BZ-Nota/issues">Report Bug</a>
        </p>
      </td>
    </tr>
  </table>
</div>

<div align="center">

[![CPP][CPP-shield]][CPP-url]
![Version](https://img.shields.io/badge/version-0.8.0-orange)
![License](https://img.shields.io/badge/license-MIT-green)
![Git LFS](https://img.shields.io/badge/Git-LFS-blue)

</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#🧐-about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#🚀-getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li>
      <a href="#✍️-usage">Usage</a>
      <ul>
        <li><a href="#basic-usage">Basic Usage</a></li>
        <li><a href="#command-line-options">Command-Line Options</a></li>
        <li><a href="#keybindings">Keybindings</a></li>
        <li><a href="#system-clipboard">System Clipboard</a></li>
      </ul>
    </li>
    <li><a href="#🖥️-tested-terminals">Tested Terminals</a></li>
    <li><a href="#📅-planned-features">Planned features</a></li>
    <li><a href="#🤝-contributing">Contributing</a></li>
    <li><a href="#📝-license">License</a></li>
    <li><a href="#📧-contact">Contact</a></li>
    <li><a href="#🙏-acknowledgments">Acknowledgments</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## 🧐 About The Project

BZ-Nota is a terminal-based text editor that aims to provide a modern and efficient editing experience in the terminal. It's designed for developers and power users who want the simplicity and keybindings of editors like Micro, combined with a powerful, integrated dashboard, inspired by `btop`.

Key Features:
*   **Intuitive Editing**: Familiar keybindings and a focus on simplicity.
*   **Modern TUI**: Clear and modern looks with emoji support.
*   **High Performance**: Built with C++ for speed and low resource usage.
*   **Minimal Resource Usage**: Uses less than 6MB on an empty file and close to zero CPU usage when editing.
*   **Safe Renaming & Saving**: Prompts before overwriting existing files and privilege elevation to save files requiring elevated permissions.

(Privilege elevation is currently being worked on)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Built With

*   [![CPP][CPP-shield]][CPP-url]
*   [![ftxui][ftxui-shield]][ftxui-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## 🚀 Getting Started

To get a local copy up and running, follow these simple steps.

### Prerequisites

You'll need a modern C++ compiler and CMake to build BZ-Nota. Installing the standard build tools is sufficient for most builds.

**Note:** Pre-packaged versions will be available upon reaching version 1.0.

*   **C++ Compiler**: Recommended — install `build-essential` (includes `g++`, `gcc`, and `make`)
*   **Alternative compilers**: Clang or MSVC are also supported
*   **CMake**: Version 3.10 or higher

**System Clipboard Support:**

*   **Linux (X11)**: `xclip` or `xsel`
*   **Linux (Wayland)**: `wl-clipboard`
*   **macOS**: Built-in clipboard commands (no installation needed) — Not tested yet.
*   **Windows**: Built-in clipboard commands (`clip` for copy, PowerShell's `Get-Clipboard` for paste) — Won't be implemented.

Example (Debian/Ubuntu):

```sh
sudo apt-get update
sudo apt-get install build-essential cmake
```

For other platforms, use the equivalent package manager (e.g., `brew`, `pacman`, etc.).

### Installation

1.  Clone the repo (including submodules)
    ```sh
    git clone --recursive https://github.com/BZ-Interactive/BZ-Nota.git
    ```

    If you already cloned the repository without `--recursive`, initialize submodules with:
    ```sh
    git submodule update --init --recursive
    ```

2.  Create a build directory
    ```sh
    cd BZ-Nota
    mkdir build && cd build
    ```
3.  Configure and build the project
    ```sh
    cmake ..
    make
    ```
4.  (Optional) Install to your system
    ```sh
    sudo make install
    ```
    This will install the `bznota` executable to your system's binary directory (typically `/usr/local/bin`), making it available system-wide.

5.  Run the editor
    ```sh
    # If installed
    bznota
    
    # Or run directly from build directory
    ./bznota
    ```

**Note:** macOS systems follow the same steps.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- USAGE EXAMPLES -->
## ✍️ Usage

if intalled you can run it from anywhere:

```sh
bznota <options> <filename>
```
if not installed run from local folder:

```sh
./bznota <options> <filename>
```

### Basic Usage

Run the editor without a file path, it will create but not save a new file "Untitled". File can be renamed via F2 during editing.

```sh
bznota
```

To launch the editor with a pre-existing file, run the executable with the file path:

```sh
bznota <filename>
```

**Options:**

*   `-h`, `--help` — Show usage and option explanations
*   `-d`, `--debug` — Enable debug mode (displays key sequence information in the status bar)
*   `-v`, `--version` — Display version information and exit
*   `-l`, `--license` — Display license information and exit
*   `--splash`, `--logo` — Display Unicode ANSI Logo and exit

**Examples:**

```sh
# Open a file for editing
bznota example.txt

# Display version information
bznota -v

# Open with debug mode enabled
bznota -d example.txt
```

### Keybindings

**File Operations:**
*   `Ctrl+S` — Save file
*   `Ctrl+Q` — Quit (prompts if unsaved changes)
*   `F2` — Rename file (interactive)

**Editing:**
*   `Ctrl+A` — Select All
*   `Ctrl+C` — Copy to system clipboard
*   `Ctrl+X` — Cut to system clipboard
*   `Ctrl+V` — Paste from system clipboard
*   `Ctrl+Insert` / `Shift+Insert` — Traditional clipboard shortcuts (alternative)
*   `Ctrl+Shift+C` / `Ctrl+Shift+V` — Modern terminal clipboard (Alacritty, WezTerm)
*   `Ctrl+Z` — Undo
*   `Ctrl+Y` — Redo
*   `Tab` — Insert tab
*   `Shift+Tab` — Remove leading tab (unindent)

**Formatting**

*   `Alt+B` — Bold
*   `Alt+I` — Italic
*   `Alt+U` — Underline
*   `Alt+T` — Strikethrough

All work on selection, can use this to write formatted or apply any format to a pre-written text.

All can be applied on top of each other.

**Editor Modes (F7):**
*   `BASIC` — Plain text, no markdown parsing (shows raw **bold** markers)
*   `FANCY` — Markdown parsing enabled (**bold**, *italic*, etc. rendered live)
*   `CODE` — Syntax highlighting, no markdown (Will be implemented after 1.0 relase, may be removed)
*   `DOCUMENT` — FANCY + grammar features (Will be implemented after 1.0 relase, may be removed)

**Appearance (F8):**
*   `F8` — Toggle between dark mode (default) and light mode

**Navigation:**
*   `Arrow Keys` — Move cursor
*   `Shift+Arrow` — Select text while moving
*   `Ctrl+Arrow` — Jump by word
*   `Ctrl+Shift+Arrow` — Select by word (Alacritty)
*   `Alt+Shift+Arrow` — Select by word (alternative)
*   `Home` — Jump to start of line (smart: toggles between first non-whitespace and column 0)
*   `End` — Jump to end of line

**Line Operations:**
*   `Ctrl+O` — Insert blank line above
*   `Ctrl+K` — Insert blank line below

**Note:** Uses Kitty Keyboard Protocol. However, some keybindings may still be intercepted by your terminal emulator. If none of the alternative keys work for you disable conflicting terminal shortcuts.

**System Clipboard:** The system clipboard feature requires external tools (`xclip`/`xsel` for X11, `wl-clipboard` for Wayland, built-in on macOS). See [CLIPBOARD.md](CLIPBOARD.md) for detailed setup instructions.

_For more examples, please refer to the [Documentation](https://github.com/BZ-Interactive/BZ-Nota)_

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## 🖥️ Tested Terminals

Below are terminals that are tested and their known quirks.

Linux:
- alacritty — **Works perfectly**.
- Konsole — **Works perfectly**.
- gnome-terminal — **Works perfectly**.
- kitty — `Ctrl+Shift+Arrow` won't work; use `Alt+Shift+Arrow` instead.
- XTerm and UXterm — **Works perfectly**, switches to unicode symbols (fixed/courier).

macOS:
- To be tested.

Windows:
- **Not supported** — Windows support will not be implemented.

<p align="right">(<a href="#readme-top">back to top</a>)</p>


## 📅 Planned features

These are short-term items planned as next priorities; the editor will focus on core editing and the dashboard first.

- Bullet Point Integration
- Privilege Elevation similar to **[Micro](https://micro-editor.github.io/)**
- User customization for UI colors
- Limited user customization for controls
- PgUp / PgDn support for navigation
- Search, Find and Replace, including Batch Replace
- Mouse Support
<p align="right">(<a href="#readme-top">back to top</a>)</p>

## 📖 Documentation

- **[Technical Design Document (PDF)](https://media.githubusercontent.com/media/BZ-Interactive/BZ-Nota/refs/heads/main/Documents/BZ-Nota_TDD.pdf)**
- **[TDD Source (.tex)](https://raw.githubusercontent.com/BZ-Interactive/BZ-Nota/refs/heads/main/Documents/BZ-Nota_TDD.tex)**
<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTRIBUTING -->
## 🤝 Contributing

Contributions are welcome! Feel free to:

- Submit feature requests via [Issues](https://github.com/BZ-Interactive/BZ-Nota/issues)
- Report bugs via [Issues](https://github.com/BZ-Interactive/BZ-Nota/issues)
- Create pull requests

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->
## 📝 License

Distributed under the MIT License. See `LICENSE` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTACT -->
## 📧 Contact

[![Email](https://img.shields.io/badge/-barkin.zorlu.bz@gmail.com-D14836?style=for-the-badge&logo=gmail&logoColor=white)](mailto:barkin.zorlu.bz@gmail.com)

[![GitHub](https://img.shields.io/badge/-@ZorluBarkin-181717?style=for-the-badge&logo=github&logoColor=white)](https://github.com/ZorluBarkin)
[![LinkedIn](https://img.shields.io/badge/linkedin-0A66C2?style=for-the-badge)](https://www.linkedin.com/in/barkin-zorlu)

Project Link: [https://github.com/BZ-Interactive/BZ-Nota](https://github.com/BZ-Interactive/BZ-Nota)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ACKNOWLEDGMENTS -->
## 🙏 Acknowledgments

*   [Micro Text Editor](https://github.com/zyedidia/micro)
*   [btop++](https://github.com/aristocratos/btop)
*   [Img Shields](https://shields.io)
*   [Choose an Open Source License](https://choosealicense.com)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: www.linkedin.com/in/barkin-zorlu
[CPP-shield]: https://img.shields.io/badge/C++(23)-015B97?style=flat-square&logo=cplusplus&logoColor=ffffff
[CPP-url]: https://isocpp.org/
[ftxui-shield]: https://img.shields.io/badge/ftxui-UI-blue
[ftxui-url]: https://github.com/ArthurSonzogni/ftxui
<a name="readme-top"></a>
