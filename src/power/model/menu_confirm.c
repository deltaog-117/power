#include "menu_confirm.h"
#include <stdio.h>
#include <string.h>

static const char *const destructive_commands[] = {
    "poweroff", "reboot", "logout"
};

int menu_command_is_destructive(const char *command) {
    if (command == NULL) {
        return 0;
    }
    for (size_t i = 0; i < sizeof(destructive_commands) / sizeof(destructive_commands[0]); i++) {
        if (strcmp(command, destructive_commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int menu_confirm_label(const char *action_label, char *out, size_t out_size) {
    if (action_label == NULL || out == NULL || out_size == 0) {
        return -1;
    }
    int written = snprintf(out, out_size, "Confirm %s", action_label);
    if (written < 0 || (size_t)written >= out_size) {
        out[0] = '\0';
        return -1;
    }
    return 0;
}
