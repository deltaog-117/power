# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- (None yet – this is the current stable version)

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
