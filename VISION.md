# Vision for power

---

## The Problem

On Linux, there is no single, consistent command to suspend, power off, reboot, lock the screen, or log out. Every desktop environment, window manager, and minimal setup has its own bespoke incantation – `systemctl suspend`, `loginctl lock-session`, `i3lock`, `gnome-session-quit --logout`, and so on. Users have to remember which commands work in which context, and scripts break when the environment changes. This fragmentation makes simple system power actions unnecessarily complicated.

---

## The Solution

`power` provides a unified, environment‑agnostic command‑line interface for the five most common system power operations: suspend, poweroff, reboot, lock, and logout. It automatically probes the current session (desktop environment, window manager, or plain X11) and executes the appropriate method, falling back intelligently when needed. If something fails, `power` gives you a clear error message and suggests next steps. It is written in C, with a modular, feature‑first architecture that keeps the codebase small, readable, and easy to extend.

---

## Guiding Principles

1. **Environment Agnosticism** – `power` works on any Linux system with a standard C library and the usual system interfaces, regardless of init system (systemd or not) or desktop environment.

2. **Clarity on Failure** – Every operation that cannot be performed returns a detailed, actionable error message. No silent failures.

3. **Modularity** – Each power operation is isolated in its own module. Deleting a feature folder should not break the rest of the application.

4. **Observability** – Critical actions are logged so that users and developers can trace exactly what `power` attempted.

5. **Professional Engineering** – The code follows modern practices: type‑safe design, property‑based testing, performance benchmarks, and structured logging – all while keeping the tool lightweight.

---

## Who This Is For

- **Linux power users** who want a single, predictable command for system power actions, regardless of the desktop environment they're using.
- **Window manager enthusiasts** (i3, bspwm, herbstluftwm, etc.) who often lack integrated session management and need a reliable, scriptable utility.
- **System administrators** who write deployment scripts and need a consistent interface across different distros and setups.
- **Developers and hobbyists** who want to learn or hack on a clean C project with a modern, modular architecture.

---

## What This Is NOT

- Not a system daemon – `power` is a one‑shot CLI tool, not a background service.
- Not a GUI – it provides no graphical interface; use your DE's native tools or a launcher for that.
- Not a replacement for `systemctl` – if you need fine‑grained control over systemd units, use `systemctl` directly.
- Not a session manager – it does not start or stop user sessions; it only performs power and lock actions on the current session.
- Not a cross‑platform tool – it is Linux‑only (no BSD or macOS support, at least for now).

---

## Why This Matters

In a world where Linux desktop environments come and go, `power` provides a stable anchor. It gives you back control over your machine's power state without forcing you to relearn a new set of commands every time you switch window managers. By keeping the code small, modular, and well‑documented, `power` also serves as an example of how to write systems‑level C tools that are both reliable and approachable.
