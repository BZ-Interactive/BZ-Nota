<!-- Improved README.md generation -->
<br />
<div align="center">
  <a href="https://github.com/BZ-Interactive/BZ-Nota">
    <!-- <img src="images/logo.png" alt="Logo" width="80" height="80"> -->
  </a>

<h3 align="center">BZ-Nota</h3>

  <p align="center">
    A high-performance, terminal-based text editor built on C++. It combines the intuitive editing of Micro with a modern, btop-style dashboard interface for a modern TUI experience.
    <br />
    <a href="https://github.com/BZ-Interactive/BZ-Nota"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/BZ-Interactive/BZ-Nota/releases">See Releases</a>
    ·
    <a href="https://github.com/BZ-Interactive/BZ-Nota/issues">Request Feature</a>
    ·
    <a href="https://github.com/BZ-Interactive/BZ-Nota/issues">Report Bug</a>
  </p>
</div>

<div align="center">

[![CPP][CPP-shield]][CPP-url]
![Version](https://img.shields.io/badge/version-0.0-orange)
![License](https://img.shields.io/badge/license-MIT-green)

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
    <li><a href="#✍️-usage">Usage</a></li>
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
*   **Modern TUI**: A dashboard for file navigation, project statistics, and more.
*   **High Performance**: Built with C++ for speed and low resource usage.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Built With

*   [![CPP][CPP-shield]][CPP-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## 🚀 Getting Started

To get a local copy up and running, follow these simple steps.

### Prerequisites

You'll need a modern C++ compiler and CMake to build BZ-Nota. Installing the standard build tools is sufficient for most builds.

**Note:** Pre-packaged versions will be available upon reaching version 1.0.

*   **C++ Compiler (GCC)**: Recommended — install `build-essential` and `g++`
*   **Alternative compilers**: Clang or MSVC are also supported
*   **CMake**: Version 3.10 or higher
*   **Optional**: Additional libraries required by your chosen TUI/terminal UI libraries — install as needed

Example (Debian/Ubuntu):

```sh
sudo apt-get update
sudo apt-get install build-essential g++ cmake
```

For other platforms, use the equivalent package manager (e.g., `brew`, `pacman`, etc.).

### Installation

1.  Clone the repo
    ```sh
    git clone https://github.com/BZ-Interactive/BZ-Nota.git
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
4.  The executable will be in the `build` directory.
    ```sh
    ./bz-nota
    ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- USAGE EXAMPLES -->
## ✍️ Usage

To launch the editor, run the executable with an optional file path:

```sh
./bz-nota [file_to_open]
```

_For more examples, please refer to the [Documentation](https://github.com/BZ-Interactive/BZ-Nota)_

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## 📅 Planned features (post-initial release)

These are short-term items planned after the initial release; the editor will focus on core editing and the dashboard first.

* **Syntax highlighting & language support** — basic highlighting for common languages
* **Dashboard widgets (configurable)** — file tree, buffer list, project stats
* **Keybinding customization & simple themes** — user-friendly defaults with later customization options
* **Session restore** — restore open files/session on startup

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

Barkın Zorlu - barkin.zorlu.bz@gmail.com

[![LinkedIn][linkedin-shield]][linkedin-url]

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
[CPP-shield]: https://img.shields.io/badge/C++-17-blue
[CPP-url]: https://isocpp.org/
<a name="readme-top"></a>
