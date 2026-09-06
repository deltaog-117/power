# Roadmap: power

This document outlines the future direction of `power`, a unified CLI for system power operations.
Items are organized by priority, not by timeline.

**Current version:** 0.5.0

---

## ✅ Completed (Milestones Achieved)

- ✅ Basic command set: `suspend`, `poweroff`, `reboot`, `lock`, `logout`
- ✅ Shortcut aliases (`sp`, `sleep`, `off`, `shutdown`, `sd`, `restart`, `rb`, `out`, `exit`, `log`)
- ✅ Fallback mechanism for screen lockers (i3lock → GNOME → KDE → xlock → loginctl)
- ✅ Fallback mechanism for logout (multiple WMs/DEs → `pkill -KILL`)
- ✅ Simple help / usage screen
- ✅ Manual sync before suspend/poweroff/reboot
- ✅ Skeleton project scaffold (feature-first, modular structure)
- ✅ Core philosophy documented in VISION.md

---

## 🔥 High Priority (Critical)

- **Refactor monolithic `power.c` into modular scaffold** – Split logic into `model/` (command definitions), `service/` (orchestration), and `repository/` (low-level OS calls). This is the foundation for all future improvements.
- **Implement structured logging** – Replace raw `printf`/`perror` with a proper logging module in `shared/logging/` (support log levels: ERROR, WARN, INFO, DEBUG) so users can trace what `power` attempted.
- **Define proper error types** – Create a unified `PowerStatus` enum (instead of returning raw `int`) with descriptive error codes and messages, integrated with the logging system.
- **Create a robust Makefile** – Provide targets: `all`, `clean`, `install` (to `/usr/local/bin`), `uninstall`, and `test` (to run the unit tests).
- **Add basic unit tests** – At minimum, test the fallback ordering logic for lockers and logout methods without actually executing system commands (use mocks / stubs).

---

## 🟡 Medium Priority (Important)

- **User configuration file** – Support `~/.config/power.conf` (or `~/.powerrc`) to allow users to:
  - Customise the priority order of screen lockers.
  - Customise the priority order of logout methods.
  - Set default operation (e.g., `default=lock` if no argument is given).
- **`--version` flag** – Display the current version and build date.
- **Improved error messages** – Move beyond simple `perror()` and provide context‑aware suggestions (e.g., `power: lock: no screen locker found. Try installing i3lock or gnome-screensaver.`).
- **Systemd/logind integration as primary** – If `logind` is available, use it for suspend/poweroff/reboot/lock instead of falling back to raw syscalls. This improves compatibility with modern distros.
- **Man page** – Write a proper `power.1` man page and include it in the install target.

---

## 🟢 Low Priority (Nice‑to‑Have)

- **Hibernate support** – Add `hibernate` command (write to `/sys/power/state` with `disk`).
- **Hybrid‑sleep support** – Add `hybrid-sleep` command (suspend to both RAM and disk).
- **Desktop notifications** – Use `notify-send` to inform the user when a power action is about to happen or has completed (e.g., "System suspended").
- **Coloured output** – Use ANSI colours to distinguish success (green), warnings (yellow), and errors (red) in the terminal.
- **Bash/Zsh completion** – Provide completion scripts for `power` commands and their aliases.

---

## 🔭 Long‑Term Vision (Exploratory)

- **DBus API** – Expose `power`'s capabilities via a D-Bus interface so that other applications (e.g., system trays, launchers) can call it programmatically.
- **Port to FreeBSD/OpenBSD** – Adapt the repository layer to use BSD equivalents (`apm`, `shutdown`, `lock` utilities) while keeping the same user interface.
- **Plugin system** – Allow users to add custom methods for locking or logging out (e.g., `~/.power/lockers/my-custom-locker.sh`) without modifying the C code.
