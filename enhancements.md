# 🚀 Power Tool - Feature Roadmap

## 🔋 Advanced Power Features
- **Hybrid Sleep** - Suspend to RAM + save to disk (safe if power lost, great for desktops with UPS)
- **Suspend-then-Hibernate** - Suspend first, then auto-hibernate after timeout (ideal for laptops on battery)
- **Reboot to UEFI** - Restart directly to BIOS/UEFI firmware settings (no more key mashing!)
- **Reboot to Boot Menu** - Restart to boot selection screen (for dual-boot systems)
- **Wake-up Timer** - Schedule wake from suspend/hibernate (automated tasks, downloads)
- **Suspend with Delay** - Suspend after X minutes (delayed sleep)


## 👤 User & Session Management
- **Switch User** - Return to login screen, keep session running in background
- **Kill Session** - Force logout (emergency escape when normal logout freezes)
- ✅ **Lock + Suspend** - Lock screen then suspend (security + power saving in one command) — implemented via `lock_before_suspend` config option
- **Lock + Logout** - Lock then logout (secure logout)


## 🔧 System Diagnostics
- **Status** - Show supported states, swap info, screen lockers available, uptime (what works on this system)
- **Active Sessions** - Show who's logged in (multi-user systems)
- **Uptime** - Show system uptime (check when last rebooted)
- **Battery Status** - Show battery percentage (for laptops)
- **Running Processes** - Check for critical processes (prevent suspend during important work)


## ⚙️ Configuration & Personalization
- **Config File** - `~/.powerrc` for user preferences (custom shortcuts, defaults)
- **System Config** - `/etc/power.conf` for system-wide settings (for administrators)
- **Default Command** - What runs when no command is given (e.g., `power` = `power status`)
- **Custom Lockers** - User-specified screen lockers (custom scripts)
- **Pre/Post Hooks** - Run scripts before/after actions (save work, send notifications)


## 🖥️ User Experience
- **Countdown Timer** - Delay with cancel option (prevent accidents)
- **Interactive Mode** - Choose action from a menu (great for new users)
- **Verbose Mode** - `-v` flag for detailed output (debugging)
- **Quiet Mode** - `-q` flag for no output (for scripts)
- **Color Output** - Warnings in red, success in green (better readability)
- **Desktop Notification** - Popup alerts before suspend (keep user informed)
- **JSON Output** - `--json` flag for scripting (integration with other tools)


## 🛡️ Safety & Validation
- **Network Check** - Don't suspend if transferring files (prevent interrupting downloads)
- **Battery Check** - Only suspend if battery > threshold (laptop/server safety)
- **CPU Load Check** - Don't suspend if system is busy (servers, rendering)
- **Process Check** - Don't suspend if specific apps are running (critical work)
- **Confirm Prompt** - Ask "Are you sure?" before destructive actions (safety)
- **Undo/Resume** - Cancel pending suspend (fix mistakes)


## 📦 Distribution & Packaging
- **Makefile** - Build, install, uninstall targets (easy deployment)
- **Man Page** - `man power` documentation (professional polish)
- **Shell Completion** - Tab completion for bash/zsh (faster typing)
- **Packaging** - PKGBUILD (Arch), deb (Debian), RPM (Fedora) (distribution packages)
- **Version Flag** - `--version` shows version info (updates)


## 🌐 Network & Remote
- **Remote Suspend** - Suspend via SSH (remote management)
- **Wake-on-LAN** - Send WOL packet (wake sleeping machines)
- **SSH Check** - Don't suspend if SSH connected (prevent interrupting remote sessions)


## 🧪 Testing & Maintenance
- **Self-Test** - Test if all features work (verify installation)
- **Debug Mode** - Detailed logs (troubleshooting)
- **Dry Run** - `--dry-run` flag shows what would happen without executing (safety check)


## 📋 Command Reference (Current)
| Command | Shortcuts | Description | Requires Sudo? |
|---------|-----------|-------------|----------------|
| `suspend` | `sleep`, `sp` | Suspend to RAM (sleep) | ✅ Yes |
| `hibernate` | `hb` | Suspend to disk (hibernate) | ✅ Yes |
| `poweroff` | `off`, `shutdown` | Power off the system | ✅ Yes |
| `reboot` | `restart`, `rb` | Reboot the system | ✅ Yes |
| `lock` | (none) | Lock the screen | ❌ No |
| `logout` | `out`, `exit` | Log out of the session | ❌ No |
| `help` | `-h`, `--help` | Show usage information | ❌ No |


## 🎯 Priority Features to Add Next
1. **Status Command** - Show supported states, lockers, swap, uptime
2. ~~**Lock + Suspend Combo** - One command for security + power saving~~ ✅ Done
3. **Countdown Timer** - Prevent accidental poweroff/suspend
4. ~~**Config File** - User preferences (`~/.powerrc`)~~ ✅ Done (`~/.config/power.conf`)
5. **Hybrid Sleep** - Suspend + hibernate combined for desktop safety


## 📝 Notes
- All features should work **cross-distro** (Debian, Arch, Fedora, etc.)
- All features should work **cross-DE/WM** (i3, AwesomeWM, GNOME, KDE, XFCE, etc.)
- Keep the binary **statically compiled** for maximum portability
- Maintain **simple, clean error messages** for users
- Preserve **backward compatibility** with existing commands

---

*Last Updated: 2026-08-12*
