# power

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![C99](https://img.shields.io/badge/C-99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](http://makeapullrequest.com)

**Unified CLI for Linux system power operations – suspend, poweroff, reboot, lock, logout.**

---

## 💡 About

On Linux, there is no single, consistent command for system power actions. Every desktop environment, window manager, and minimal setup has its own incantation – `systemctl suspend`, `loginctl lock-session`, `i3lock`, `gnome-session-quit --logout`, and so on. Users have to remember which commands work in which context, and scripts break when the environment changes.

**power** provides a unified, environment‑agnostic command‑line interface for the five most common system power operations. It automatically probes the current session (desktop environment, window manager, or plain X11) and executes the appropriate method, falling back intelligently when needed. If something fails, `power` gives you a clear error message and suggests next steps.

Whether you're a minimalist running `dwm`, a GNOME user, or a system administrator writing deployment scripts, `power` gives you a single, reliable command.

---

## ✨ Features

- 🔹 **Six core commands** – `suspend`, `hibernate`, `poweroff`, `reboot`, `lock`, `logout`
- 🔹 **Intelligent fallback** – Tries multiple lockers and logout methods (i3, GNOME, KDE, XFCE, and more)
- 🔹 **Shortcut aliases** – `sp` for suspend, `off` for poweroff, `rb` for reboot, `out` for logout
- 🔹 **Structured logging** – Timestamped logs with levels (DEBUG, INFO, WARN, ERROR, CRITICAL)
- 🔹 **Modular architecture** – Feature‑first, vertically sliced design for easy maintenance
- 🔹 **Small & fast** – Written in C99, minimal dependencies
- 🔹 **Version flag** – `--version` or `-v` to display build information
- 🔹 **Systemd/logind integration** – Uses `loginctl` when available, falls back gracefully
- 🔹 **User configuration** – Customise locker/logout order and default command via `~/.config/power.conf`
- 🔹 **Lock before sleep** – Screen locks before suspending or hibernating by default; the action is refused if locking fails

---

## 📋 Requirements

- **Language / Runtime** – C compiler (gcc or clang) with C99 support
- **Build system** – `make`
- **Operating system** – Linux (kernel with `/sys/power/state` and `reboot()` syscall)
- **Optional** (for screen locking/logout):
  - `i3lock`, `gnome-screensaver`, `kscreenlocker`, `xlock`, or `loginctl`
  - Various window manager/DE logout commands (i3, bspwm, GNOME, KDE, XFCE, etc.)

---

## 📦 Installation

### Build from source

```bash
# Clone the repository
git clone https://github.com/deltaog-117/power.git
cd power

# Build
make

# (Optional) Install system‑wide
sudo make install
```

### Verify installation

```bash
power help
```

---

## 🚀 Usage

### Basic commands

```bash
power suspend   # Suspend to RAM (sleep)
power hibernate # Suspend to disk (hibernate)
power poweroff  # Power off the system
power reboot    # Reboot the system
power lock      # Lock the screen
power logout    # Log out of the current session
power menu      # Show an interactive power menu (pick an action with rofi/dmenu/etc.)
power help      # Show usage
power --version # Show version information
power -v        # Show version information (short)
```

### Shortcuts

| Shortcut | Command       |
|----------|---------------|
| `sp`     | `suspend`     |
| `sleep`  | `suspend`     |
| `hb`     | `hibernate`   |
| `off`    | `poweroff`    |
| `sd`     | `poweroff`    |
| `shutdown`| `poweroff`   |
| `rb`     | `reboot`      |
| `restart`| `reboot`      |
| `out`    | `logout`      |
| `exit`   | `logout`      |
| `log`    | `logout`      |

Example:

```bash
power off   # Power off the system
power rb    # Reboot the system
```

---

## ⚙️ Configuration

### Environment Variables

| Variable           | Purpose                                                                 | Default |
|--------------------|-------------------------------------------------------------------------|---------|
| `POWER_LOG_LEVEL`  | Set logging verbosity: `DEBUG`, `INFO`, `WARN`, `ERROR`, `CRITICAL`     | `INFO`  |

Example:

```bash
POWER_LOG_LEVEL=DEBUG power lock
```

### Configuration File

`power` supports a user configuration file at `~/.config/power.conf`. This allows you to:

- **Set a default command** – if no command is provided, `power` uses this.
- **Customise locker priority order** – change which lockers are tried first.
- **Customise logout priority order** – change which logout methods are tried first.

#### Example `~/.config/power.conf`

```ini
# Default command when none is provided
default = lock

# Custom locker order (comma-separated)
locker_order = i3lock, gnome-screensaver-command -l, xlock, loginctl lock-session

# Custom logout order (comma-separated)
logout_order = i3-msg exit, bspc quit, gnome-session-quit --logout --no-prompt

# Lock the screen before suspending (default: true)
lock_before_suspend = true

# Lock the screen before hibernating (default: true)
lock_before_hibernate = true

# Custom `power menu` launcher order (comma-separated)
menu_launcher_order = rofi -dmenu -p power, wofi --dmenu --prompt power, dmenu -p power

# Confirm Poweroff, Reboot and Logout in `power menu` (default: true)
menu_confirm = true
```

If a config file is not found, `power` uses its built‑in defaults.

#### Interactive menu

`power menu` shows the six core actions (`Lock`, `Suspend`, `Hibernate`,
`Poweroff`, `Reboot`, `Logout`) through whichever launcher is available,
trying `rofi`, `wofi`, `bemenu`, `fuzzel`, then `dmenu` in that order (each
is checked with `command -v` before being launched, so a distro that only
has one of them still works out of the box). Bind it in your window manager,
e.g. for a Super+Ctrl activation:

```
# i3/sway config
bindsym $mod+Control+p exec power menu
```

Since `power menu` pipes its options to the launcher's stdin and reads the
pick back from stdout — the same "dmenu protocol" every one of those tools
already speaks — pointing `menu_launcher_order` at a custom launcher you
write yourself works too, as long as it follows that same convention.

##### Confirming destructive actions

Picking `Poweroff`, `Reboot` or `Logout` in `power menu` reopens the same launcher
with two rows, `Confirm <action>` and `Cancel`. The confirmation row is first, so a
plain `Enter` accepts it; `Cancel`, `Esc` or closing the launcher does nothing.
`Lock`, `Suspend` and `Hibernate` are recoverable and never ask. Set
`menu_confirm = false` in `power.conf` to turn this off. Direct commands such as
`power poweroff` are unaffected, so scripts and keybinds keep working.

##### Built-in X11 menu (optional)

`power` can also draw its own menu, with no external launcher needed:

```bash
sudo pacman -S libx11 libxft libxinerama   # or your distro's -dev packages
make MENU=builtin
```

It is a small borderless window centred on the monitor under the pointer.
Keys: `Up`/`Down`, `j`/`k`, `Tab`, `Ctrl+P`/`Ctrl+N` move; `1`–`6` jump to a row;
`Enter` confirms; `Esc`, `q` or `Ctrl+C` cancels. The mouse works too: click a row
to pick it, click outside to cancel. Number keys only highlight, never confirm, so
a stray keypress cannot power the machine off.

Colours default to your X resources (`*background`, `*foreground`, `*color4`,
`*color0`), so it follows your theme (pywal included). Override them with
`menu_bg`, `menu_fg`, `menu_sel_bg`, `menu_sel_fg`, `menu_border`, and set the
font with `menu_font` (a fontconfig pattern) in `power.conf`. In this build
`builtin` is tried before the external launchers; if it cannot run (Wayland
session, no X display), `power` falls back to them. A plain `make` builds
without it and needs no extra libraries.

#### Lock before sleep

By default, `power suspend` (`sp`/`sleep`) and `power hibernate` (`hb`) lock the screen
using the same locker fallback chain as `power lock` *before* sleeping. If no locker
succeeds, the action is refused rather than leaving the session unlocked and
unattended. This works on any distro/WM `power` already supports, with no dependency
on systemd sleep hooks.

Set `lock_before_suspend = false` and/or `lock_before_hibernate = false` in
`power.conf` if you'd rather suspend/hibernate without locking (e.g. on a headless or
single-user machine) — the two are controlled independently.

For screen locking on suspends triggered *outside* `power` (lid close, idle timeout),
pair `power` with [`xss-lock`](https://bitbucket.org/raymonad/xss-lock/), which listens
for the logind/elogind `PrepareForSleep` signal and works the same way on both
systemd and non-systemd (elogind-based) distros.

---

## 📁 Project Structure

```
src/
├── power/                    # Core feature (vertical slice)
│   ├── model/                # Data models (enums, status codes, config)
│   │   ├── command.h         # Command enum and parser
│   │   ├── command.c
│   │   ├── config.h          # Configuration loading
│   │   ├── config.c
│   │   ├── status.h          # PowerStatus enum and string conversion
│   │   ├── status.c
│   │   └── version.h         # Version and build date
│   ├── repository/           # Low‑level system calls
│   │   ├── system_repo.h     # suspend, poweroff, reboot, lock, logout
│   │   └── system_repo.c
│   └── service/              # Orchestration layer
│       ├── power_service.h   # Single entry point: power_service_execute()
│       └── power_service.c
├── shared/                   # Read‑only infrastructure
│   ├── logging/              # Structured logging
│   │   ├── logger.h
│   │   └── logger.c
│   └── utils/                # Generic utilities (placeholder)
│       └── string_utils.c
├── main.c                    # Thin entrypoint (arg parsing, log setup, config)
tests/
├── unit/                     # Unit tests (mirror src/)
│   └── power/
│       ├── test_command.c    # (to be written)
│       └── test_system_repo.c
└── integration/              # Integration tests
    └── power/
        └── test_power_integration.c
```

---

## 🧪 Testing

Unit tests are not yet implemented. We welcome contributions!

```bash
make test   # placeholder for now
```

---

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/amazing`).
3. Commit your changes (`git commit -m 'Add amazing feature'`).
4. Push to the branch (`git push origin feature/amazing`).
5. Open a Pull Request.

Read the [ROADMAP.md](ROADMAP.md) for planned features and priorities.

---

## 📄 License

This project is licensed under the [GNU Affero General Public License v3.0](LICENSE) – see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgements

- The open‑source community for their incredible tools and inspiration.
- Window manager and desktop environment developers who make Linux great.

---

## 💬 Questions / Support

Open an [issue](https://github.com/deltaog-117/power/issues) or reach out via email.

---

## 📜 Changelog

See the [CHANGELOG.md](CHANGELOG.md) file for version history.
