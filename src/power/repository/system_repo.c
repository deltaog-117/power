#include "system_repo.h"
#include "../model/status.h"
#include "../../shared/logging/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/reboot.h>

#define SYS_POWER_STATE "/sys/power/state"

PowerStatus system_suspend(void) {
    LOG_INFO("Suspending system...");

    sync();

    FILE *fp = fopen(SYS_POWER_STATE, "w");
    if (!fp) {
        LOG_ERROR("Failed to open %s: %s", SYS_POWER_STATE, strerror(errno));
        return POWER_STATUS_ERR_IO;
    }

    if (fprintf(fp, "mem") < 0) {
        LOG_ERROR("Failed to write to %s: %s", SYS_POWER_STATE, strerror(errno));
        fclose(fp);
        return POWER_STATUS_ERR_IO;
    }

    fclose(fp);
    return POWER_STATUS_OK;
}

PowerStatus system_poweroff(void) {
    LOG_INFO("Powering off system...");
    sync();
    reboot(RB_POWER_OFF);
    LOG_ERROR("reboot(RB_POWER_OFF) failed: %s", strerror(errno));
    return POWER_STATUS_ERR_SYSTEM;
}

PowerStatus system_reboot(void) {
    LOG_INFO("Rebooting system...");
    sync();
    reboot(RB_AUTOBOOT);
    LOG_ERROR("reboot(RB_AUTOBOOT) failed: %s", strerror(errno));
    return POWER_STATUS_ERR_SYSTEM;
}

PowerStatus system_lock(void) {
    const char *lock_cmds[] = {
        "i3lock",
        "gnome-screensaver-command -l",
        "kscreenlocker --lock",
        "xlock",
        "loginctl lock-session",
        NULL
    };

    for (int i = 0; lock_cmds[i] != NULL; i++) {
        LOG_DEBUG("Trying locker: %s", lock_cmds[i]);
        int ret = system(lock_cmds[i]);
        if (ret == 0) {
            LOG_INFO("Screen locked successfully");
            return POWER_STATUS_OK;
        }
    }

    LOG_ERROR("No screen locker found");
    return POWER_STATUS_ERR_NO_LOCKER;
}

PowerStatus system_logout(void) {
    const char *logout_cmds[] = {
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

    for (int i = 0; logout_cmds[i] != NULL; i++) {
        LOG_DEBUG("Trying logout method: %s", logout_cmds[i]);
        int ret = system(logout_cmds[i]);
        if (ret == 0) {
            LOG_INFO("Logged out successfully");
            return POWER_STATUS_OK;
        }
    }

    LOG_ERROR("No logout method found");
    return POWER_STATUS_ERR_NO_LOGOUT_METHOD;
}
