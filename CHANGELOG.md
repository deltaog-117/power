# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **User configuration file** – `~/.config/power.conf` to customise:
  - Locker priority order (`locker_order`)
  - Logout priority order (`logout_order`)
  - Default command (`default`)
- `src/power/model/config.h` and `config.c` for config parsing
- Sample config file at `config/power.conf.example`
- **Lock before suspend** – `system_suspend()` now locks the screen (via the same
  fallback chain as `power lock`) before suspending, and refuses to suspend if the
  lock fails. Controlled by `lock_before_suspend` in `power.conf` (default: `true`)
- **`hibernate` command** (alias `hb`) – suspends to disk via `loginctl hibernate`,
  falling back to writing `disk` to `/sys/power/state`. Locks the screen first by
  default, refusing to hibernate if the lock fails; controlled independently via
  `lock_before_hibernate` in `power.conf` (default: `true`)
- **`menu` command** – shows an interactive pop-up of the six core actions (lock,
  suspend, hibernate, poweroff, reboot, logout) via any dmenu-protocol launcher
  (rofi, wofi, bemenu, fuzzel, dmenu, tried in that order and detected with
  `command -v`), so it works out of the box on most distros/WMs. Configurable via
  `menu_launcher_order` in `power.conf`. Intended for a window-manager keybind
  (e.g. Super+Ctrl)
- `POWER_STATUS_ERR_NO_LAUNCHER` status for when no menu launcher is available

### Changed
- `system_lock()` and `system_logout()` now use config-specified order when available
- `main.c` loads config on startup and uses default command if set

### Fixed
- `config.c` failed to compile (`PATH_MAX` undeclared) due to a missing `<limits.h>` include

---

## [0.5.0] - 2026-09-06

### Added
- Complete modular, feature‑first architecture (vertical slices)
- Command model layer (`src/power/model/`) with `Command` enum and parser
- Unified status codes (`PowerStatus`) with human‑readable string conversion
- Repository layer (`src/power/repository/`) for low‑level system calls:
  - `system_suspend()` – writes to `/sys/power/state`
  - `system_poweroff()` – calls `reboot(RB_POWER_OFF)`
  - `system_reboot()` – calls `reboot(RB_AUTOBOOT)`
  - `system_lock()` – tries common screen lockers with fallback
  - `system_logout()` – tries common logout methods with fallback
- Service layer (`src/power/service/`) orchestrating command execution
- Structured logging module (`src/shared/logging/`) with:
  - Log levels: DEBUG, INFO, WARN, ERROR, CRITICAL
  - Timestamped output
  - Environment variable `POWER_LOG_LEVEL` for verbosity control
- Thin entrypoint (`src/main.c`) with argument parsing
- Professional Makefile with `all`, `clean`, `install`, `uninstall`, `test` targets
- Support for command aliases (sp, sleep, off, shutdown, sd, rb, restart, out, exit, log)
- Help screen with usage and shortcut list
- Doxygen‑style comments for all public functions
- Compilation with `-Wall -Wextra -Wpedantic -std=gnu99 -D_GNU_SOURCE`
- `.gitignore` for C, Make, Linux, Vim, Neovim, and VSCode

### Changed
- **Refactored** the original monolithic `power.c` (500+ lines) into a clean, modular scaffold
- **Improved** error handling with explicit status codes instead of raw return codes
- **Replaced** raw `printf`/`perror` with structured logging
- **Standardised** naming conventions (snake_case for functions/variables, UPPER_SNAKE_CASE for macros)
- **Applied** 1TBS brace style and 4‑space indentation
- **Made** headers self‑sufficient with include guards and `extern "C"` wrappers

### Fixed
- No silent failures – all operations now return explicit status codes with logged errors
- All public functions have clear parameter and return value documentation
- Memory ownership is documented (static strings, no dynamic allocations)

### Security
- No secrets are logged
- All system calls are checked for errors

---

## [0.1.0] - 2026-08-23

### Added
- Initial prototype: monolithic `power.c`
- Basic commands: suspend, poweroff, reboot, lock, logout
- Simple fallback for lockers and logout methods
- Basic help screen
- `sync()` before suspend/poweroff/reboot
