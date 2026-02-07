# System Clipboard Support

BZ-Nota now supports both internal clipboard and system clipboard operations.

## Keyboard Shortcuts

### System Clipboard
- **Ctrl+C**: Copy selection to system clipboard
- **Ctrl+V**: Paste from system clipboard  
- **Ctrl+X**: Cut selection to system clipboard

**Additional system clipboard shortcuts for compatibility:**
- **Ctrl+Insert**: Copy to system clipboard (traditional)
- **Shift+Insert**: Paste from system clipboard (traditional)
- **Ctrl+Shift+C/V**: Copy/Paste (modern terminals like Alacritty)
- **Alt+Shift+C/V**: Copy/Paste (alternative for modern terminals)

**Note:** If system clipboard tools are not available (no xclip/wl-clipboard/pbcopy installed), you'll see an error message when trying to use clipboard operations.

## Platform Support

The system clipboard feature automatically detects and uses the appropriate clipboard tool for your platform:

### Linux (X11)
Requires one of the following tools installed:
- `xclip` (recommended)
- `xsel` (fallback)

Install on Debian/Ubuntu:
```bash
sudo apt install xclip
# or
sudo apt install xsel
```

Install on Fedora/RHEL:
```bash
sudo dnf install xclip
# or
sudo dnf install xsel
```

Install on Arch:
```bash
sudo pacman -S xclip
# or
sudo pacman -S xsel
```

### Linux (Wayland)
Requires:
- `wl-clipboard` (provides `wl-copy` and `wl-paste`)

Install on Debian/Ubuntu:
```bash
sudo apt install wl-clipboard
```

Install on Fedora/RHEL:
```bash
sudo dnf install wl-clipboard
```

Install on Arch:
```bash
sudo pacman -S wl-clipboard
```

### macOS
Uses built-in `pbcopy` and `pbpaste` commands. No additional installation required.

## How It Works

1. **Detection**: The editor automatically detects your display server (X11, Wayland) and platform (Linux, macOS)
2. **Tool Selection**: Uses the appropriate clipboard tool:
   - macOS: `pbcopy`/`pbpaste`
   - Wayland: `wl-copy`/`wl-paste`
   - X11: `xclip` or `xsel`
3. **Fallback**: If system clipboard is not available, you'll see an error message, but internal clipboard will still work

## Status Messages

The editor shows status messages when using system clipboard:
- "Copied to system clipboard" - successful copy
- "Pasted N characters from system clipboard" - successful paste
- "Failed to copy to system clipboard" - no clipboard tool found
- "Failed to paste from system clipboard" - clipboard tool error
- "System clipboard is empty" - nothing to paste

## Troubleshooting

### "Failed to copy to system clipboard"
This means no clipboard tool was found. Install the appropriate tool for your platform (see above).

### Clipboard not working on Wayland
Make sure `wl-clipboard` is installed and the `WAYLAND_DISPLAY` environment variable is set.

### Clipboard not working on X11  
Make sure `xclip` or `xsel` is installed and the `DISPLAY` environment variable is set.
