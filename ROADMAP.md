# Roadmap: power

This document outlines the future direction of `power`, a unified CLI for system power operations.
Items are organized by priority, not by timeline.

**Current version:** 0.5.0

---

## ✅ Completed (Milestones Achieved)

- ✅ Basic command set: `suspend`, `hibernate`, `poweroff`, `reboot`, `lock`, `logout`
- ✅ Shortcut aliases (`sp`, `sleep`, `hb`, `off`, `shutdown`, `sd`, `restart`, `rb`, `out`, `exit`, `log`)
- ✅ Fallback mechanism for screen lockers (i3lock → GNOME → KDE → xlock → loginctl)
- ✅ Fallback mechanism for logout (multiple WMs/DEs → `pkill -KILL`)
- ✅ Simple help / usage screen
- ✅ Manual sync before suspend/poweroff/reboot
- ✅ Skeleton project scaffold (feature-first, modular structure)
- ✅ Core philosophy documented in VISION.md
- ✅ Modular refactor of `power.c` into model/service/repository layers
- ✅ Structured logging with levels and timestamp
- ✅ Unified `PowerStatus` error enum
- ✅ Robust Makefile with install/uninstall/test targets
- ✅ `--version` / `-v` flag
- ✅ **Systemd/logind integration** – use `loginctl` as primary method for suspend, poweroff, reboot, and lock
- ✅ **Improved error messages** – user‑friendly suggestions for common failures
- ✅ **User configuration file** – `~/.config/power.conf` for custom locker/logout order and default command
- ✅ **Lock before suspend** – `power suspend` locks the screen first by default (`lock_before_suspend` in config), refusing to suspend if locking fails
- ✅ **Hibernate support** – `power hibernate` (`hb`) suspends to disk via `loginctl hibernate`, falling back to `/sys/power/state`; locks the screen first by default (`lock_before_hibernate` in config)

---

## 🔥 High Priority (Critical)

- **Add basic unit tests** – At minimum, test the fallback ordering logic for lockers and logout methods without actually executing system commands (use mocks / stubs).

---

## 🟡 Medium Priority (Important)

- **Man page** – Write a proper `power.1` man page and include it in the install target.

---

## 🟢 Low Priority (Nice‑to‑Have)

- **Hybrid‑sleep support** – Add `hybrid-sleep` command (suspend to both RAM and disk).
- **Desktop notifications** – Use `notify-send` to inform the user when a power action is about to happen or has completed (e.g., "System suspended").
- **Coloured output** – Use ANSI colours to distinguish success (green), warnings (yellow), and errors (red) in the terminal.
- **Bash/Zsh completion** – Provide completion scripts for `power` commands and their aliases.

---

## 🔭 Long‑Term Vision (Exploratory)

- **DBus API** – Expose `power`'s capabilities via a D-Bus interface so that other applications (e.g., system trays, launchers) can call it programmatically.
- **Port to FreeBSD/OpenBSD** – Adapt the repository layer to use BSD equivalents (`apm`, `shutdown`, `lock` utilities) while keeping the same user interface.
- **Plugin system** – Allow users to add custom methods for locking or logging out (e.g., `~/.power/lockers/my-custom-locker.sh`) without modifying the C code.
