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

- 🔹 **Five core commands** – `suspend`, `poweroff`, `reboot`, `lock`, `logout`
- 🔹 **Intelligent fallback** – Tries multiple lockers and logout methods (i3, GNOME, KDE, XFCE, and more)
- 🔹 **Shortcut aliases** – `sp` for suspend, `off` for poweroff, `rb` for reboot, `out` for logout
- 🔹 **Structured logging** – Timestamped logs with levels (DEBUG, INFO, WARN, ERROR, CRITICAL)
- 🔹 **Modular architecture** – Feature‑first, vertically sliced design for easy maintenance
- 🔹 **Small & fast** – Written in C99, minimal dependencies
- 🔹 **Version flag** – `--version` or `-v` to display build information
- 🔹 **Systemd/logind integration** – Uses `loginctl` when available, falls back gracefully
- 🔹 **User configuration** – Customise locker/logout order and default command via `~/.config/power.conf`

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
power poweroff  # Power off the system
power reboot    # Reboot the system
power lock      # Lock the screen
power logout    # Log out of the current session
power help      # Show usage
power --version # Show version information
power -v        # Show version information (short)
```

### Shortcuts

| Shortcut | Command       |
|----------|---------------|
| `sp`     | `suspend`     |
| `sleep`  | `suspend`     |
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
```

If a config file is not found, `power` uses its built‑in defaults.

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
