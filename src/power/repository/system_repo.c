#include "system_repo.h"
#include "../model/status.h"
#include "../model/config.h"
#include "../../shared/logging/logger.h"
#ifdef POWER_MENU_X11
#include "menu_x11.h"
#endif
#ifdef POWER_MENU_WAYLAND
#include "menu_wayland.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/reboot.h>

#define SYS_POWER_STATE "/sys/power/state"

/* Global config (set by main) */
static PowerConfig *g_config = NULL;

void system_repo_set_config(PowerConfig *config) {
    g_config = config;
}

static int logind_available_cached = -1;

static int logind_available(void) {
    if (logind_available_cached != -1) {
        return logind_available_cached;
    }
    int ret = system("loginctl --version >/dev/null 2>&1");
    logind_available_cached = (ret == 0);
    return logind_available_cached;
}

static PowerStatus try_logind_action(const char *action) {
    if (!logind_available()) {
        return POWER_STATUS_ERR_SYSTEM;
    }

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "loginctl %s", action);
    LOG_DEBUG("Trying logind: %s", cmd);

    int ret = system(cmd);
    if (ret == 0) {
        LOG_INFO("loginctl %s succeeded", action);
        return POWER_STATUS_OK;
    } else {
        LOG_WARN("loginctl %s failed (ret=%d), falling back", action, ret);
        return POWER_STATUS_ERR_SYSTEM;
    }
}

int system_repo_menu_confirm_enabled(void) {
    /* Unset means on: an unconfigured menu must not power off in one step. */
    return g_config && g_config->menu_confirm != -1 ? g_config->menu_confirm : 1;
}

static int should_lock(int configured) {
    return configured != -1 ? configured : 1; /* default: lock the screen first */
}

static PowerStatus lock_or_refuse(const char *action, const char *config_key, int should) {
    if (!should) {
        return POWER_STATUS_OK;
    }

    PowerStatus lock_status = system_lock();
    if (lock_status != POWER_STATUS_OK) {
        LOG_ERROR("Refusing to %s: screen lock failed (%s)", action, power_status_to_string(lock_status));
        LOG_ERROR("Suggestion: install a screen locker, fix locker_order in power.conf, or set %s = false to %s without locking.",
                   config_key, action);
    }
    return lock_status;
}

static PowerStatus write_power_state(const char *state, const char *action) {
    LOG_INFO("Falling back to /sys/power/state for %s", action);
    sync();

    FILE *fp = fopen(SYS_POWER_STATE, "w");
    if (!fp) {
        LOG_ERROR("Failed to open %s: %s", SYS_POWER_STATE, strerror(errno));
        LOG_ERROR("Suggestion: Run with sudo or ensure write access to /sys/power/state.");
        return POWER_STATUS_ERR_IO;
    }

    if (fprintf(fp, "%s", state) < 0) {
        LOG_ERROR("Failed to write to %s: %s", SYS_POWER_STATE, strerror(errno));
        LOG_ERROR("Suggestion: Check kernel support for %s; try 'cat /sys/power/state' to see available states.", action);
        fclose(fp);
        return POWER_STATUS_ERR_IO;
    }

    fclose(fp);
    return POWER_STATUS_OK;
}

PowerStatus system_suspend(void) {
    PowerStatus lock_status = lock_or_refuse("suspend", "lock_before_suspend",
        should_lock(g_config ? g_config->lock_before_suspend : -1));
    if (lock_status != POWER_STATUS_OK) {
        return lock_status;
    }

    if (try_logind_action("suspend") == POWER_STATUS_OK) {
        return POWER_STATUS_OK;
    }

    return write_power_state("mem", "suspend");
}

PowerStatus system_hibernate(void) {
    PowerStatus lock_status = lock_or_refuse("hibernate", "lock_before_hibernate",
        should_lock(g_config ? g_config->lock_before_hibernate : -1));
    if (lock_status != POWER_STATUS_OK) {
        return lock_status;
    }

    if (try_logind_action("hibernate") == POWER_STATUS_OK) {
        return POWER_STATUS_OK;
    }

    return write_power_state("disk", "hibernate");
}

PowerStatus system_poweroff(void) {
    if (try_logind_action("poweroff") == POWER_STATUS_OK) {
        return POWER_STATUS_OK;
    }

    LOG_INFO("Falling back to reboot(RB_POWER_OFF)");
    sync();
    reboot(RB_POWER_OFF);
    LOG_ERROR("reboot(RB_POWER_OFF) failed: %s", strerror(errno));
    LOG_ERROR("Suggestion: Try using 'shutdown -h now' or 'systemctl poweroff' directly.");
    return POWER_STATUS_ERR_SYSTEM;
}

PowerStatus system_reboot(void) {
    if (try_logind_action("reboot") == POWER_STATUS_OK) {
        return POWER_STATUS_OK;
    }

    LOG_INFO("Falling back to reboot(RB_AUTOBOOT)");
    sync();
    reboot(RB_AUTOBOOT);
    LOG_ERROR("reboot(RB_AUTOBOOT) failed: %s", strerror(errno));
    LOG_ERROR("Suggestion: Try using 'shutdown -r now' or 'systemctl reboot' directly.");
    return POWER_STATUS_ERR_SYSTEM;
}

PowerStatus system_lock(void) {
    /* Default locker order */
    const char *default_lockers[] = {
        "i3lock",
        "gnome-screensaver-command -l",
        "kscreenlocker --lock",
        "xlock",
        "loginctl lock-session",
        NULL
    };

    /* Use config if available and non-empty */
    const char **lockers = NULL;
    int count = 0;

    if (g_config && g_config->locker_count > 0) {
        lockers = (const char **)g_config->locker_order;
        count = g_config->locker_count;
        LOG_DEBUG("Using config-specified locker order (%d entries)", count);
    } else {
        lockers = default_lockers;
        while (default_lockers[count] != NULL) count++;
        LOG_DEBUG("Using default locker order (%d entries)", count);
    }

    for (int i = 0; i < count; i++) {
        LOG_DEBUG("Trying locker: %s", lockers[i]);
        int ret = system(lockers[i]);
        if (ret == 0) {
            LOG_INFO("Screen locked successfully");
            return POWER_STATUS_OK;
        }
    }

    LOG_ERROR("No screen locker found.");
    LOG_ERROR("Suggestion: Install i3lock, gnome-screensaver, kscreensaver, xlock, or ensure logind is available.");
    return POWER_STATUS_ERR_NO_LOCKER;
}

static int program_available(const char *launcher_cmd) {
    char prog[128];
    size_t i = 0;

    while (launcher_cmd[i] != '\0' && launcher_cmd[i] != ' ' && i < sizeof(prog) - 1) {
        prog[i] = launcher_cmd[i];
        i++;
    }
    prog[i] = '\0';

    if (prog[0] == '\0') {
        return 0;
    }

    char check[160];
    snprintf(check, sizeof(check), "command -v %s >/dev/null 2>&1", prog);
    return system(check) == 0;
}

#ifdef POWER_BUILTIN_MENU
/* Each compiled-in backend declines (ERR_NO_LAUNCHER) when the session is not
 * its kind, so trying Wayland first and X11 second picks the right one. */
static PowerStatus run_builtin_menu(const char *const options[], int count,
                                    char *selection, size_t selection_size) {
    PowerStatus status = POWER_STATUS_ERR_NO_LAUNCHER;
#ifdef POWER_MENU_WAYLAND
    status = menu_wayland_run(options, count, g_config, selection, selection_size);
    if (status != POWER_STATUS_ERR_NO_LAUNCHER) {
        return status;
    }
#endif
#ifdef POWER_MENU_X11
    status = menu_x11_run(options, count, g_config, selection, selection_size);
#endif
    return status;
}
#endif

PowerStatus system_repo_run_launcher(const char *const options[], int count,
                                      char *selection, size_t selection_size) {
    if (options == NULL || selection == NULL || selection_size == 0) {
        return POWER_STATUS_ERR_NULL_POINTER;
    }
    selection[0] = '\0';

    /* Default launcher order: covers dwm/i3/sway-style setups (dmenu),
     * general X11/Wayland rofi/wofi users, and the lighter bemenu/fuzzel. */
    const char *default_launchers[] = {
#ifdef POWER_BUILTIN_MENU
        "builtin",
#endif
        "rofi -dmenu -p power",
        "wofi --dmenu --prompt power",
        "bemenu -p power",
        "fuzzel --dmenu --prompt=power>",
        "dmenu -p power",
        NULL
    };

    const char **launchers = NULL;
    int launcher_count = 0;

    if (g_config && g_config->menu_launcher_count > 0) {
        launchers = (const char **)g_config->menu_launcher_order;
        launcher_count = g_config->menu_launcher_count;
        LOG_DEBUG("Using config-specified menu launcher order (%d entries)", launcher_count);
    } else {
        launchers = default_launchers;
        while (default_launchers[launcher_count] != NULL) launcher_count++;
        LOG_DEBUG("Using default menu launcher order (%d entries)", launcher_count);
    }

    for (int i = 0; i < launcher_count; i++) {
        /* "builtin" names the menu compiled into power itself; it is not a
         * program, so it skips the PATH check and the popen pipeline. */
        if (strcmp(launchers[i], "builtin") == 0) {
#ifdef POWER_BUILTIN_MENU
            PowerStatus builtin_status =
                run_builtin_menu(options, count, selection, selection_size);
            if (builtin_status == POWER_STATUS_OK) {
                return POWER_STATUS_OK;
            }
            LOG_DEBUG("Built-in menu unavailable; trying the next launcher");
#else
            LOG_DEBUG("Built-in menu not compiled in (build with MENU=builtin)");
#endif
            continue;
        }

        if (!program_available(launchers[i])) {
            LOG_DEBUG("Menu launcher not found: %s", launchers[i]);
            continue;
        }

        /* Build: printf '%s\n' 'opt1' 'opt2' ... | <launcher> */
        char items[1024] = "printf '%s\\n'";
        size_t len = strlen(items);
        for (int j = 0; j < count; j++) {
            int written = snprintf(items + len, sizeof(items) - len, " '%s'", options[j]);
            if (written < 0 || (size_t)written >= sizeof(items) - len) {
                LOG_ERROR("Menu option list too long for launcher pipeline");
                return POWER_STATUS_ERR_UNKNOWN;
            }
            len += (size_t)written;
        }

        char full_cmd[1536];
        snprintf(full_cmd, sizeof(full_cmd), "%s | %s", items, launchers[i]);

        LOG_DEBUG("Running menu launcher: %s", launchers[i]);
        FILE *fp = popen(full_cmd, "r");
        if (!fp) {
            LOG_WARN("Failed to launch %s: %s", launchers[i], strerror(errno));
            continue;
        }

        if (fgets(selection, selection_size, fp) != NULL) {
            size_t sel_len = strlen(selection);
            while (sel_len > 0 && (selection[sel_len - 1] == '\n' || selection[sel_len - 1] == '\r')) {
                selection[--sel_len] = '\0';
            }
        }

        pclose(fp);
        return POWER_STATUS_OK;
    }

    LOG_ERROR("No menu launcher found.");
    LOG_ERROR("Suggestion: install rofi, wofi, bemenu, fuzzel, or dmenu, build with MENU=builtin, or set menu_launcher_order in power.conf.");
    return POWER_STATUS_ERR_NO_LAUNCHER;
}

PowerStatus system_logout(void) {
    /* Default logout order */
    const char *default_logout[] = {
        "i3-msg exit",
        "bspc quit",
        "herbstclient quit",
        "qtile cmd-obj -o cmd -f shutdown",
        "awesome-client 'awesome.quit()'",
        "echo 'quit' | dvtm",
        "screen -X quit",
        "gnome-session-quit --logout --no-prompt",
        "xfce4-session-logout --logout",
        "mate-session-save --logout",
        "cinnamon-session-quit --logout",
        "enlightenment_exit",
        "lxde-logout",
        "lxqt-session --logout",
        "budgie-session --logout",
        "plasma-exit",
        "pkill -KILL -u $USER 2>/dev/null",
        NULL
    };

    /* Use config if available and non-empty */
    const char **logout_methods = NULL;
    int count = 0;

    if (g_config && g_config->logout_count > 0) {
        logout_methods = (const char **)g_config->logout_order;
        count = g_config->logout_count;
        LOG_DEBUG("Using config-specified logout order (%d entries)", count);
    } else {
        logout_methods = default_logout;
        while (default_logout[count] != NULL) count++;
        LOG_DEBUG("Using default logout order (%d entries)", count);
    }

    for (int i = 0; i < count; i++) {
        LOG_DEBUG("Trying logout method: %s", logout_methods[i]);
        int ret = system(logout_methods[i]);
        if (ret == 0) {
            LOG_INFO("Logged out successfully");
            return POWER_STATUS_OK;
        }
    }

    LOG_ERROR("No logout method found.");
    LOG_ERROR("Suggestion: Ensure a supported window manager or desktop environment is running.");
    LOG_ERROR("For minimal setups, manually use 'pkill -KILL -u $USER' (force logout).");
    return POWER_STATUS_ERR_NO_LOGOUT_METHOD;
}
