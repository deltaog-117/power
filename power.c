#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/reboot.h>

void print_usage(const char *progname) {
    printf("Usage: %s [command]\n", progname);
    printf("Commands:\n");
    printf("  suspend   - Suspend to RAM (sleep)\n");
    printf("  poweroff  - Power off the system\n");
    printf("  reboot    - Reboot the system\n");
    printf("  lock      - Lock the screen\n");
    printf("  logout    - Log out of the current session\n");
    printf("  help      - Show this help message\n");
    printf("\nShortcuts:\n");
    printf("  sp, sleep    - Same as suspend\n");
    printf("  off, shutdown, sd - Same as poweroff\n");
    printf("  restart, rb  - Same as reboot\n");
    printf("  out, exit, log - Same as logout\n");
}

int do_suspend(void) {
    sync();
    FILE *fp = fopen("/sys/power/state", "w");
    if (!fp) {
        perror("Error opening /sys/power/state");
        return 1;
    }
    printf("Suspending system...\n");
    fflush(stdout);
    fprintf(fp, "mem");
    fclose(fp);
    // If we get here, suspend failed
    return 1;
}

int do_poweroff(void) {
    printf("Powering off system...\n");
    fflush(stdout);
    sync();
    reboot(RB_POWER_OFF);
    // If we get here, poweroff failed
    return 1;
}

int do_reboot(void) {
    printf("Rebooting system...\n");
    fflush(stdout);
    sync();
    reboot(RB_AUTOBOOT);
    // If we get here, reboot failed
    return 1;
}

int do_lock(void) {
    // Try common screen lockers in order of preference
    const char *lock_cmds[] = {
        "i3lock",                      // i3 window manager
        "gnome-screensaver-command -l", // GNOME
        "kscreenlocker --lock",        // KDE Plasma
        "xlock",                       // Generic X11
        "loginctl lock-session",       // systemd-logind
        NULL
    };
    
    for (int i = 0; lock_cmds[i] != NULL; i++) {
        if (system(lock_cmds[i]) == 0) {
            return 0;  // Success
        }
    }
    
    fprintf(stderr, "Error: No screen locker found\n");
    fprintf(stderr, "Try installing i3lock, gnome-screensaver, or xlock\n");
    return 1;
}

int do_logout(void) {
    // Try common logout methods in order of preference
    const char *logout_cmds[] = {
        // Window Managers
        "i3-msg exit",                 // i3
        "bspc quit",                   // bspwm
        "herbstclient quit",           // herbstluftwm
        "qtile cmd-obj -o cmd -f shutdown", // Qtile
        "awesome-client 'awesome.quit()'", // Awesome WM
        "echo 'quit' | dvtm",          // dvtm
        "screen -X quit",              // GNU Screen
        
        // Desktop Environments
        "gnome-session-quit --logout --no-prompt", // GNOME
        "xfce4-session-logout --logout",           // XFCE
        "mate-session-save --logout",              // MATE
        "cinnamon-session-quit --logout",          // Cinnamon
        "enlightenment_exit",                      // Enlightenment
        "lxde-logout",                             // LXDE
        "lxqt-session --logout",                   // LXQt
        "budgie-session --logout",                 // Budgie
        "plasma-exit",                             // KDE Plasma
        
        // Generic X11
        "pkill -KILL -u $USER 2>/dev/null",        // Force kill (last resort)
        NULL
    };
    
    for (int i = 0; logout_cmds[i] != NULL; i++) {
        if (system(logout_cmds[i]) == 0) {
            return 0;  // Success
        }
    }
    
    fprintf(stderr, "Error: No logout method found\n");
    fprintf(stderr, "Try one of these commands manually:\n");
    fprintf(stderr, "  - i3-msg exit\n");
    fprintf(stderr, "  - gnome-session-quit --logout\n");
    fprintf(stderr, "  - xfce4-session-logout --logout\n");
    fprintf(stderr, "  - pkill -KILL -u $USER (force logout)\n");
    return 1;
}

int main(int argc, char *argv[]) {
    const char *cmd = "suspend";
    
    if (argc > 1) {
        cmd = argv[1];
    }
    
    if (strcmp(cmd, "suspend") == 0
    || strcmp(cmd, "sleep") == 0 
    || strcmp(cmd, "sp") == 0) {
        return do_suspend();
        
    } else if (strcmp(cmd, "poweroff") == 0
    || strcmp(cmd, "off") == 0
    || strcmp(cmd, "shutdown") == 0
    || strcmp(cmd, "sd") == 0
    || strcmp(cmd, "po") == 0) { 
        return do_poweroff();
        
    } else if (strcmp(cmd, "reboot") == 0
    || strcmp(cmd, "restart") == 0
    || strcmp(cmd, "rb") == 0) {
        return do_reboot();
        
    } else if (strcmp(cmd, "lock") == 0) {
        return do_lock();
        
    } else if (strcmp(cmd, "logout") == 0
    || strcmp(cmd, "out") == 0
    || strcmp(cmd, "log") == 0
    || strcmp(cmd, "exit") == 0) {
        return do_logout();
        
    } else if (strcmp(cmd, "help") == 0 
    || strcmp(cmd, "-h") == 0 
    || strcmp(cmd, "--help") == 0) {
        print_usage(argv[0]);
        return 0;
        
    } else {
        fprintf(stderr, "Unknown command: %s\n", cmd);
        print_usage(argv[0]);
        return 1;
    }
}